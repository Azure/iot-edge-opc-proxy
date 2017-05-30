#!/bin/bash

# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

current_root=$(pwd)
tool_root=$(cd "$(dirname "$0")" && pwd)
me=$(basename "$0")
repo_root=$(cd "$tool_root/../../.." && pwd)

share_root=`mktemp -t rootXXXXXXXX`
build_config=Release

fuzzer_index=
fuzzer_jobs=0
fuzzer_reset=0
fuzzer_interval=3h
fuzzer_detailed=1
fuzzer_use_asan=0
fuzzer_no_push=0

role=0
worker_flags=""
git_uri=
git_sync=0

# manager instance name
instance_name=$HOSTNAME

# project name and executable to fuzz
mode="json"
executable="protofuzz"

usage ()
{
    echo "$me [options]"
    echo "start options"
    echo " -x --xtrace              print a trace of each command."
    echo " -e --executable <value>  [${executable}] Name of the executable to fuzz."
    echo " -m --mode <value>        [${mode}] Name of mode, doubles as subfolder with seed tests and passed as"
    echo "                          first arg to the executable. Project folder will be (executable)-(mode)."
    echo " -r --repo-root <value>   [${share_root}] Performs a sync with a git repo in @ <value> (pull/push)."
    echo "                          If not .git repo, prompts user for url. If empty, uses folder but no git."
    echo "    --no-push             Does not perform a git push during sync."
    echo "    --name <value>        [${instance_name}] Name of the instance."
    echo " -C --config <value>      [${build_config}] Builds this configuration (e.g. Debug, Release) to fuzz."
    echo " -j --jobs <value>        [${fuzzer_jobs}] Number of docker images to run. 0 == auto."
    echo " -i --interval <value>    [${fuzzer_interval}] Amount of time between resyncing (e.g. 2m, 24h, etc.)."
    echo " -l --lightweight         Perform lightweight minimization of corpus skipping tmin."
    echo "    --use-asan            Uses address sanitizer and not hardening and libdislocator. "
    echo "control options:"
    echo "    --destroy             Stops all workers and manager and removes all docker containers."
    echo " -R --reattach            Reattach terminal to manager.  Useful when session is disconnected."
    exit 1
}

# -----------------------------------------------------------------------------
# -- Parse arguments
# -----------------------------------------------------------------------------
process_args ()
{
    # Note that we use `"$@"' to let each command-line parameter expand to a
    # separate word. The quotes around `$@' are essential!
    # We need TEMP as the `eval set --' would nuke the return value of getopt.
    TEMP=`getopt -o xC:j:i:n:r:le:m:R \
          -l xtrace,config:,jobs:,repo-root:,worker,manager,resume,destroy,interval:,index:,name:,use-asan,lightweight,no-push,executable:,mode:,reattach,reset \
         -- "$@"`

    if [ $? != 0 ]
    then
        echo "Failed to parse options"
        usage
        exit 1
    fi

    # Note the quotes around `$TEMP': they are essential!
    eval set -- "$TEMP"

    while true; do
    case "$1" in
        -x | --xtrace)
            set -x
            worker_flags="${worker_flags} $1"
            shift ;;
        -C | --config)
            build_config="$2"
            shift 2 ;;
        -j | --jobs)
            fuzzer_jobs="$2"
            shift 2 ;;
        -e | --executable)
            executable="$2"
            shift 2 ;;
        -m | --mode)
            mode="$2"
            shift 2 ;;
        --worker)
            role=1
            shift ;;
        --manager)
            role=2
            shift ;;
        --destroy)
            role=3
            shift ;;
        --reset)
            fuzzer_reset=1
            shift ;;
        -R | --reattach)
            role=4
            shift ;;
        -r | --repo-root)
            share_root="$2"
            shift 2 ;;
        -i | --interval)
            fuzzer_interval="$2"
            shift 2 ;;
        -n | --index)
            fuzzer_index="$2"
            shift 2 ;;
        --name)
            instance_name="$2"
            shift 2 ;;
        --use-asan)
            fuzzer_use_asan=1
            worker_flags="${worker_flags} $1"
            shift ;;
        --no-push)
            fuzzer_no_push=1
            worker_flags="${worker_flags} $1"
            shift ;;
        -l | --lightweight)
            fuzzer_detailed=0
            worker_flags="${worker_flags} $1"
            shift ;;
        --)
            shift
            break ;; # Terminate the while loop at the last option
        *)
            usage
            shift ;
    esac
    done
}

#
# Cleanup all project related docker containers
#
cleanup() 
{
    # cleanup
    if [ $(docker ps -aqf "label=${project}" | wc -l ) -gt 0 ]; then 
        echo "Stopping all containers for project ${project}..."
        docker stop $(docker ps -aqf "label=${project}") \
            || echo "   failed ..."
        echo "Removing containers..."
        docker rm -f $(docker ps -aqf "label=${project}") || return $?
    fi
    return 0
}

#
# Here we print a summary same as afl-whatsup, but supporting docker workers 
# (no kill pid).
#
whatsup()
{
    echo "Summary:"
    echo "========"
    echo "                Time : $(date -u)"

    cur_time=`date +%s`
    TMP=`mktemp -t .afl-whatsup-XXXXXXXX` || return 1
                   fuzzer_count=0
                     total_time=0
                    total_execs=0
                      total_eps=0
                  total_crashes=0
                     total_pfav=0
                  total_pending=0
    for fuzzer in `find ${share_root}/${project} -maxdepth 2 -iname fuzzer_stats | sort`; do
        sed 's/^command_line.*$/_skip:1/;s/[ ]*:[ ]*/="/;s/$/"/' "$fuzzer" >"$TMP"
        . "$TMP"
                   runtime_unix=$((cur_time - start_time))
                   runtime_days=$((runtime_unix / 60 / 60 / 24))
                  runtime_hours=$(((runtime_unix / 60 / 60) % 24))
                   fuzzer_count=$((fuzzer_count + 1))
                       exec_sec=$((execs_done / runtime_unix))
                   path_percent=$((cur_path * 100 / paths_total))
                     total_time=$((total_time + runtime_unix))
                      total_eps=$((total_eps + exec_sec))
                    total_execs=$((total_execs + execs_done))
                  total_crashes=$((total_crashes + unique_crashes))
                  total_pending=$((total_pending + pending_total))
                     total_pfav=$((total_pfav + pending_favs))
    done
                     total_days=$((total_time / 60 / 60 / 24))
                      total_hrs=$(((total_time / 60 / 60) % 24))

    echo "       Fuzzers found : $fuzzer_count"
    echo "      Total run time : $total_days days, $total_hrs hours"
    echo "         Total execs : $((total_execs / 1000 / 1000)) million"
    echo "    Cumulative speed : $total_eps execs/sec"
    echo "       Pending paths : $total_pfav faves, $total_pending total"
    if [ "$fuzzer_count" -gt "1" ]; then
    echo "  Pending per fuzzer : $((total_pfav/fuzzer_count)) faves,"
    echo "                       $((total_pending/fuzzer_count)) total (on average)"
    fi
    echo "       Crashes found : $total_crashes locally unique"
    rm -f "$TMP"
    return 0
}

#
# Here we start as many workers as there are cores on the machine as well
# as one manager instance to manage them.  The workers will do the actual
# fuzz testing. The manager will periodically resync as specified with -i.
#
# Workers are run in a special container environment, i.e. an image which 
# has all tools installed, including afl, afl-utils, cwtriage, exploitable,
# etc. Here first the base image, then the code to fuzz test is built.
#
start()
{
    # start mode - set up containers, sync dir, and start workers / manager
    echo "building ${build_config} container images ..."

    # build afl environment
    cd "${repo_root}/bld/docker" > /dev/null
    docker build -f Dockerfile.ubuntu-xenial.afl -t afl-env . || return $?

    # build clone into afl environment
    cd "${repo_root}"
    AFL_MODE="AFL_HARDEN"
    if [ ${fuzzer_use_asan} != 0 ]; then
        AFL_MODE="AFL_USE_ASAN"
    fi
    ( cat <<END
FROM afl-env
COPY / /repo
ENV ${AFL_MODE} 1
ENV AFL_NO_BUILTIN 1
RUN ./repo/bld/docker/docker-build.sh --cl -ggdb -C $build_config --skip-unittests
WORKDIR /repo
END
    ) > tmp.Dockerfile
    docker build -f tmp.Dockerfile -t afl-${project} . || return $?
    rm -f tmp.Dockerfile

    mkdir -p ${share_root}
    if [ $git_sync == 0 ]; then
        if [ "$(ls -A $share_root)" ] ; then
            echo "${share_root} is not empty, using its content..."
        else
            # If not a repo, and git uri not provided, prompt
            if [ -z "$git_uri" ]; then
                echo "${share_root} is empty, but not a git repo."
                read -p "Do you want to add a remote repo? Y/n [n]" git_init_prompt
                if [ "$git_init_prompt" == "Y" ] ; then
                    read -p "Enter git remote uri:" git_uri
                fi
            fi
            if [ -n "$git_uri" ]; then
                echo "Cloning $git_uri"
                git clone "${git_uri}" "${share_root}" || return $?
                git_sync=1
            fi
        fi
    fi

    # Create or sync sync dir and if needed seed initial test cases
    cd "${share_root}" > /dev/null
    share_root=$(pwd)
    rm -rf testcases
    mkdir -pm777 testcases
    if [ -e ${tool_root}/${mode} ] && [ "$(ls -A ${tool_root}/${mode})" ]; then 
        cp ${tool_root}/${mode}/* ${share_root}/testcases || return $?
    else
        echo "0" > "${share_root}/testcases/seed.in"
    fi
    echo "Creating sync dir"
    mkdir -pm777 ${project}
    if [ $git_sync != 0 ]; then
        git pull --commit --no-edit --no-stat || return $?
    fi
    if [ $fuzzer_reset == 0 ] && [ -e "${project}" ] && [ "$(ls -A ${project})" ]; then 
        echo "Found previous sync directory ${share_root}/${project}!"
        read -p "Do you want to reset workers to use only initial set of test cases? Y/n [n]: " seed_prompt
    else
        seed_prompt="Y"
    fi
    if [ "$seed_prompt" == "Y" ] ; then
        echo "WARN: Resetting all workers to use ${share_root}/testcases as seed!"
        fuzzer_reset=1
        ls -a testcases
    fi

    for (( fuzzer_index=0 ; fuzzer_index < $fuzzer_jobs ; fuzzer_index++ )) ; do
        echo "Starting worker s_${instance_name}_${fuzzer_index} mounted to ${share_root}:/share..."

        queue_folder="${share_root}/${project}/${instance_name}_${fuzzer_index}"
        if [ $fuzzer_reset != 0 ]; then
            rm -rf "${queue_folder}/queue" || echo "Failed resetting queue!"
            rm -rf "${queue_folder}/_resume" || echo "Failed resetting _resume!"
        fi

        docker run -l ${project}=worker -itd --restart=on-failure:2 --privileged=true \
            -v ${share_root}:/share \
            -h s_${instance_name}_${fuzzer_index} --entrypoint=/repo/bld/test/fuzz/${me} \
                afl-${project} \
                    --worker -C $build_config $worker_flags --name ${instance_name} \
                    -e ${executable} -m ${mode} -r /share -n $fuzzer_index \
                || return $?
    done

    echo "Starting manager m_${instance_name} mounted to ${share_root}:/share..."
    # Give access to docker daemon socket so manager can start/stop workers
    docker run -l ${project}=manager -it --restart=always --privileged=true \
        -v /var/run/docker.sock:/var/run/docker.sock -v ${share_root}:/share \
        -h m_${instance_name} --entrypoint=/repo/bld/test/fuzz/${me} \
            afl-${project} \
                --manager -C $build_config $worker_flags --name ${instance_name} \
                -e ${executable} -m ${mode} -r /share -i $fuzzer_interval \
        || return $?

    return 0
}

#
# Worker - performs actual fuzz testing. Runs afl-fuzz as 
# master instance -n=0, or as support -n>0.
#
work()
{
    if [ -z $fuzzer_index ]; then
        echo "missing index!  -- needs -n arg."
        return 1
    fi
    echo "Running ${build_config} build of fuzzer in ${repo_root}/build/cmake/${build_config}/bin..."
    if [ $fuzzer_index == 0 ] ; then
        m_or_s="-M"
    else
        m_or_s="-S"
    fi

    ( echo core >/proc/sys/kernel/core_pattern ) || echo "Failed writing core pattern!"

    queue_folder="${share_root}/${project}/${instance_name}_${fuzzer_index}/queue"
    if [ -e ${queue_folder} ] && [ "$(ls -A ${queue_folder})" ]; then 
        fuzzer_input="-i-"
    else
        fuzzer_input="-i ${share_root}/testcases"
    fi
    if [ $fuzzer_use_asan == 0 ] ; then 
        export AFL_PRELOAD=/usr/local/lib/afl/libdislocator.so
    fi

    cd ${repo_root}/build/cmake/${build_config}/bin > /dev/null \
        || return $?
    export AFL_SKIP_CPUFREQ=1
    afl-fuzz \
        ${fuzzer_input} -o $share_root/${project} $m_or_s ${instance_name}_${fuzzer_index} \
            -m none \
            -- ${repo_root}/build/cmake/${build_config}/bin/${executable} -${mode} @@ \
            || return $?
    return 0
}

#
# The manager stops all containers and then runs, crashwalk
# triage and corpus minimization.  Finally it syncs its corpus
# with other managers to seed workers, and upon completion 
# all workers are restarted.
#
manage()
{
    if [ $git_sync != 0 ]; then
        cd "${share_root}/${project}" > /dev/null
        ( cat <<END
**/queue.*/**
**/hangs.*/**
**/crashes.*/**
**/_resume/**
**/._out_tmp/**
**/.state/**
**/.cur_input
END
        ) > ".gitignore"
        echo -e "\033[1mInitialize git client instance...\033[0m"
        git config --global user.email "${instance_name}@${project}.se"
        git config --global user.name "${instance_name}"
        git config --global push.default simple
        git config --global merge.renamelimit 999999
        git add ".gitignore" \
            || echo " ... failed."
        git commit --allow-empty -q -m "${project}: ${instance_name} start @ $(date -u)" \
            || echo " ... failed."
        if [ $fuzzer_no_push == 0 ] ; then
            git config credential.helper 'store' 
            git pull --commit --no-edit --no-stat || echo " ... failed."
            git push || fuzzer_no_push=1
        fi
        if [ $fuzzer_no_push != 0 ] ; then
            echo -e "\033[1mWill not perform a push of corpus to your remote, so you need to do it manually!\033[0m"
        fi
    fi
 
    start_option=
 
    cd ${repo_root}/build/cmake/${build_config}/bin > /dev/null
    for testcase in `find ${share_root}/testcases -maxdepth 1 -type f | sort`; do
        echo -e "\033[1mTesting instrumentation for ${executable} -${mode} ${testcase}...\033[0m"
        TMP=`mktemp -t .afl-showmap-XXXXXXXX` || return 1
        afl-showmap -o "$TMP" -m none \
            -- ${repo_root}/build/cmake/${build_config}/bin/${executable} -${mode} ${testcase}
        if [ $? != 0 ]; then
            echo -e "\033[1mError: Showmap failed - this indicates an issue in the instrumentation!\033[0m"
        fi
        rm -f "$TMP"
    done
    
    fuzzer_consolidate=1
    for (( ; ; )) ; do
        echo "Waiting ${fuzzer_interval}..."
        sleep ${fuzzer_interval}

        # do resync - pause all fuzzers, consolidate corpus, unpause
        if [ $(docker ps -qf "label=${project}=worker" | wc -l ) -eq 0 ]; then 
            echo -e "\033[1mError: No more workers running - this indicates an issue!\033[0m"
            start_option="-ai"
        fi

        if [ $fuzzer_consolidate != 0 ] && [ "$start_option" != "-ai" ]; then
            echo -e "\033[1mStopping all workers...\033[0m"
            docker stop $(docker ps -qf "label=${project}=worker") \
                || echo " ... failed."
        fi

        # Print whats up...
        whatsup

        if [ $fuzzer_consolidate != 0 ]; then
            # Create working folder structure
            (
                rm -rf "/work_tmp"
                mkdir "/work_tmp"
                cd "/work_tmp" > /dev/null
                mkdir "crashes"
                mkdir "hangs"
                mkdir "${project}"
                cd "${project}" > /dev/null
                mkdir old_corpus
                mkdir new_corpus
            ) || return $?
            
            echo -e "\033[1mCopying files from ${instance_name} sync folders to working folder...\033[0m"
            cd "${share_root}" > /dev/null
            # copy all files from project instance queues
            cd "${share_root}/${project}" > /dev/null
            cp -r ${instance_name}_* /work_tmp/${project}/old_corpus \
                || return $?

            echo -e "\033[1mCollecting new hangs...\033[0m"
            for fuzzer in `find /work_tmp/${project}/old_corpus -mindepth 1 -maxdepth 1 -type d` ; do 
                if [ -e "${fuzzer}/hangs" ] && [ "$(ls -A ${fuzzer}/hangs)" ]; then 
                    cd "${fuzzer}/hangs" > /dev/null
                    echo "Collecting hangs in ${fuzzer}/hangs ... "
                    cp * "/work_tmp/hangs" \
                        || echo "  ... failed."
                else
                    echo "No hangs in ${fuzzer}/hangs"
                fi
            done
            mkdir -p "${share_root}/${project}_crashes/hangs"
            if [ "$(ls -A /work_tmp/hangs)" ]; then 
                cd "/work_tmp/hangs" > /dev/null
                # Sanitize hang file names so they can be used on Windows.
                rename 's/\:/_/g' * > /dev/null
                cp -uf * "${share_root}/${project}_crashes/hangs" \
                    || echo "  ... failed."
            fi

            # Run local triage now on new crashes
            mkdir -p "${share_root}/${project}_crashes/crashes"
            cd "${share_root}/${project}_crashes" > /dev/null
            echo -e "\033[1mCollecting new crashes and run triage...\033[0m"
            afl-collect -d ./${instance_name}_ac.db -e /work_tmp/gdb_script -j $fuzzer_jobs \
                /work_tmp/${project}/old_corpus /work_tmp/crashes \
                    -- ${repo_root}/build/cmake/${build_config}/bin/${executable} -${mode} @@ \
                || echo "  ... failed."
            if [ "$(ls -A /work_tmp/crashes)" ]; then 
                if [ $fuzzer_detailed != 0 ]; then
                    cwtriage -root /work_tmp/crashes -workers $fuzzer_jobs \
                        -seendb ${share_root}/${project}_crashes/${instance_name}_cw.db \
                            -- ${repo_root}/build/cmake/${build_config}/bin/${executable} -${mode} @@ \
                        || echo "  ... failed."
                    cwdump ${share_root}/${project}_crashes/${instance_name}_cw.db \
                        > ${share_root}/${project}_crashes/${instance_name}_triage.txt \
                        || echo "  ... failed."
                fi
                cd "/work_tmp/crashes" > /dev/null
                # Sanitize crash file names so they can be used on Windows.
                rename 's/\:/_/g' * > /dev/null
                cp -uf * "${share_root}/${project}_crashes/crashes" \
                    || echo "  ... failed."
            fi

            echo -e "\033[1mConsolidating corpus mounted in /work_tmp/${project}...\033[0m"
            afl_min_mode="--cmin"
            afl_min_out=/work_tmp/${project}/new_corpus.cmin
            if [ $fuzzer_detailed != 0 ]; then
                afl_min_mode="$afl_min_mode --tmin"
                afl_min_out="${afl_min_out}.tmin"
            fi
            afl-minimize $afl_min_mode -j $fuzzer_jobs --reseed \
                -c /work_tmp/${project}/new_corpus /work_tmp/${project}/old_corpus \
                    -- ${repo_root}/build/cmake/${build_config}/bin/${executable} -${mode} @@ 

            if [ $? == 0 ] && [ -e "${afl_min_out}" ] && [ "$(ls -A ${afl_min_out})" ]; then 
                echo -e "\033[1mRestoring corpus in ${share_root}:/share from working folder...\033[0m"
                cd "${share_root}/${project}" > /dev/null
                mkdir -p "backup_${instance_name}"
                mv ${instance_name}_* "backup_${instance_name}" \
                    || echo "  ... failed."
                cd "/work_tmp/${project}/old_corpus/" > /dev/null
                mv ${instance_name}_* "${share_root}/${project}" \
                    || return $?
                echo "Cleaning up seed cases and temporary files..."
                cd "${share_root}" > /dev/null
                rm -rf "${project}/backup_${instance_name}" \
                    || echo "  ... failed."

                for fuzzer in `find ${share_root}/${project} -mindepth 1 -maxdepth 1 -type d` ; do 
                    cd "${fuzzer}" > /dev/null
                    # cleanup backup folders
                    rm -rf queue.*
                    rm -rf crashes.*
                    rm -rf hangs.*
                done
            else
                echo -e "\033[1mError: Minimization did not yield new corpus!\033[0m"
            fi
        fi

        # sync with git repo
        cd "${share_root}" > /dev/null
        if [ $git_sync != 0 ]; then
            echo -e "\033[1mAdd corpus to repo...\033[0m"

            git add "${project}/${instance_name}_*/**"
            if [ $? != 0 ]; then 
                echo -e "\033[1mNothing to add... - this could indicate a problem!!!\033[0m"
                start_option="-ai"
            fi
            git add "${project}/${instance_name}_*/*" > /dev/null 
            git add "${project}/${instance_name}_*/.*/**" > /dev/null 
            git add "${project}_crashes/**" \
                || echo " ... failed."
            git add "${project}_crashes/*" \
                || echo " ... failed."
            git add "*" \
                || echo " ... failed."
            git commit --allow-empty -q -m "${project}: ${instance_name} sync @ $(date -u)" \
                || echo " ... failed."
            echo "Pull remote corpus..."
            git pull --commit --no-edit --no-stat \
                || echo " ... failed."
            if [ $fuzzer_no_push == 0 ]; then
                echo -e "\033[1mPush local changes...\033[0m"
                git push \
                    || echo " ... failed."
            fi
        fi

        # Restart all workers
        cd "${share_root}" > /dev/null
        if [ $fuzzer_consolidate != 0 ] && [ "$start_option" != "-ai" ]; then
            echo "Restarting all workers..."
            docker start $(docker ps -aqf "label=${project}=worker") \
                || exit $?
        else
            if [ "$start_option" == "-ai" ]; then
                echo -e "\033[1mStarting all workers attached - detach using C+PQ to continue\033[0m"
                for worker in `docker ps -aqf "label=${project}=worker"` ; do 
                    docker start -ai $worker || exit $?
                done
            fi
        fi
        start_option=
    done
    return 0
}

#
# Useful if manager output got detached.
#
reattach()
{
    docker attach $(docker ps -aqf "label=${project}=manager")
    return $?
}

#
# Main entry point - executes setup, manager or worker role tasks.
#
main()
{
    project="${executable}-${mode}"
    echo "== Fuzz project ${project} =="

    if [ $fuzzer_jobs == 0 ]; then
        fuzzer_jobs=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
    fi

    if [ -e "${share_root}/.git" ]; then
        git_sync=1
    fi

    if [ $role == 2 ]; then
        manage || return $?
        return 0
    fi

    set -e

    if [ $role == 1 ]; then
        work || return $?
        return 0
    fi

    if [ $role == 0 ]; then
        cleanup || return $?
        start || return $?
        return 0
    fi

    if [ $role == 3 ]; then
        cleanup || return $?
        return 0
    fi

    if [ $role == 4 ]; then
        reattach || return $?
        return 0
    fi

    echo "Bad role $role"
    return 1
}

process_args $*
main
cd "${tool_root}" > /dev/null