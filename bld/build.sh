#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

current_root=$(pwd)
repo_root=$(cd "$(dirname "$0")/.." && pwd)

run_unittests=ON
skip_dotnet=0
use_zlog=OFF
with_memcheck=OFF
use_dnssd=ON
prefer_dnssd_embedded_api=OFF
toolset=
compile_options=" "
MAKE_PARALLEL_JOBS=0

build_root="${repo_root}"/build
build_clean=0
build_os=
build_pack_only=0
build_configs=()
build_nuget_output=$build_root

usage ()
{
    echo "build.sh [options]"
    echo "options"
    echo "    --os <value>                [] Os to build on (needs Docker installed)."
    echo " -c --clean                     Build clean (Removes previous build output)."
    echo " -C --config <value>            [Debug Release] Build configuration (e.g. Debug, Release)."
    echo " -T --toolset <value>           An optional toolset to use."
    echo "    --cl <value>                Specify additional compile options to be passed to the compiler"
    echo " -o --build-root <value>        [/build] Directory in which to place all files during build."
    echo "    --use-zlog                  Use zlog as logging framework instead of xlogging."
    echo "    --use-dnssd <value>         [Yes] Sets the dnssd build option (Yes, No, Embedded)."
    echo "    --with-memcheck             Compile in memory checks."
    echo "    --skip-unittests            Skips building and executing unit tests."
    echo "    --skip-dotnet               Do not build dotnet core API and packages."
    echo "    --pack-only                 Only creates packages. (Cannot be combined with --clean)"
    echo " -n --nuget-folder <value>      [/build] Folder to use when outputting nuget packages."
    echo " -x --xtrace                    print a trace of each command."
    echo " -j --jobs <value>              Number of parallel make jobs, see description of make -j."
    echo "                                Unit test need a lot of memory. So --skip-unittests should be set,"
    echo '                                if you want to use all cores with -j`nproc`'
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
    TEMP=`getopt -o xo:C:T:cn:j: \
          -l xtrace,build-root:,config:,clean,toolset:,cl:,use-zlog,use-dnssd:,use-openssl,use-libwebsockets,with-memcheck,pack-only,skip-unittests,skip-dotnet,os:,nuget-folder:,jobs: \
         -- "$@"`

    if [ $? != 0 ]; then
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
            shift ;;
        -o | --build-root)
            build_root="$2"
            shift 2 ;;
        -C | --config)
            build_configs+=("$2")
            shift 2 ;;
        -T | --toolset)
            toolset="-T $2"
            shift 2 ;;
        --cl)
            compile_options="$2 $compile_options"
            shift 2 ;;
        -c | --clean)
            build_clean=1
            shift ;;
        --with-memcheck)
            with_memcheck=ON
            shift ;;
        --use-zlog)
            use_zlog=ON
            shift ;;
        --use-dnssd)
              if [ "${2,,}" == "no" ]; then
                use_dnssd=OFF
            elif [ "${2,,}" == "embedded" ]; then
                use_dnssd=ON
                prefer_dnssd_embedded_api=ON
            elif [ "${2,,}" == "yes" ]; then
                use_dnssd=ON
            else
                echo "Bad argument for --use-dnssd: $2"
                usage
                exit 1
            fi
            shift 2 ;;
        --use-openssl)
            shift ;;
        --use-libwebsockets)
            shift ;;
        --pack-only)
            build_pack_only=1
            shift ;;
        --skip-unittests)
            run_unittests=OFF
            shift ;;
        --skip-dotnet)
            skip_dotnet=1
            shift ;;
        --os)
            build_os="$2"
            shift 2 ;;
        -n | --nuget-folder)
            build_nuget_output="$2"
            shift 2 ;;
        -j | --jobs)
            MAKE_PARALLEL_JOBS=$2
            shift 2 ;;
        --) 
            shift
            break ;; # Terminate the while loop at the last option
        *)
            usage
            shift ;
    esac
    done
}

# -----------------------------------------------------------------------------
# -- build natively with CMAKE and run tests
# -----------------------------------------------------------------------------
native_build()
{
    if [ $build_pack_only == 0 ]; then
        echo -e "\033[1mBuilding native...\033[0m"
        for c in ${build_configs[@]}; do
            echo -e "\033[1m    ${c}...\033[0m"
            mkdir -p "${build_root}/cmake/${c}"
            cd "${build_root}/cmake/${c}" > /dev/null

            cmake $toolset -DCMAKE_BUILD_TYPE=$c -Drun_unittests:BOOL=$run_unittests \
                  -Dcompile_options_C:STRING="$compile_options" -Dcompile_options_CXX:STRING="$compile_options" \
                  -Dmem_check:BOOL=$with_memcheck -Duse_zlog:BOOL=$use_zlog -Duse_dnssd:BOOL=$use_dnssd \
                  -Dprefer_dnssd_embedded_api:BOOL=$prefer_dnssd_embedded_api -DLWS_IPV6:BOOL=ON "$repo_root" || \
                return 1
            
            CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu || nproc)
            if [ $MAKE_PARALLEL_JOBS == 0 ]; then
                MAKE_PARALLEL_JOBS=$CORES
                #
                # Make sure there is enough virtual memory on the device to handle more than one job  
                # Acquire total memory and total swap space setting them to zero in the event the command fails
                #
                MEMAR=( $(sed -n -e 's/^MemTotal:[^0-9]*\([0-9][0-9]*\).*/\1/p' -e 's/^SwapTotal:[^0-9]*\([0-9][0-9]*\).*/\1/p' /proc/meminfo) )
                [ -z "${MEMAR[0]##*[!0-9]*}" ] && MEMAR[0]=0
                [ -z "${MEMAR[1]##*[!0-9]*}" ] && MEMAR[1]=0

                let VSPACE=${MEMAR[0]}+${MEMAR[1]}
                if [ "$VSPACE" -lt "1500000" ] ; then
                    MAKE_PARALLEL_JOBS=1
                fi
            fi
            
            #
            # Start as many parallel jobs as requested by the user or auto calculated, until the load 
            # average equals the number of cores.  Be verbose if something goes wrong.
            #
            make -j$MAKE_PARALLEL_JOBS --load-average=$CORES || make VERBOSE=1 || \
                return 1
            if [ $run_unittests == OFF ]; then
                echo "Skipping unittests..."
            else
                #use doctored (-DPURIFY no-asm) openssl
                LD_LIBRARY_PATH=/usr/local/ssl/lib ctest -j $MAKE_PARALLEL_JOBS -C "$c" --output-on-failure || \
                    return 1
            fi
        done
    fi
    return 0
}

# -----------------------------------------------------------------------------
# -- build dotnet api
# -----------------------------------------------------------------------------
managed_build()
{
    if [ $skip_dotnet == 1 ]; then
        echo "Skipping dotnet..."
    else
        if dotnet --version; then
            cd "${repo_root}/api/csharp" > /dev/null
            if [ $build_pack_only == 0 ]; then
                echo -e "\033[1mBuilding dotnet...\033[0m"
                dotnet restore || exit 1
            fi
            for c in ${build_configs[@]}; do
                echo -e "\033[1m    ${c}...\033[0m"

                mkdir -p "${build_nuget_output}/${c}"
                mkdir -p "${build_root}/${c}"

                for f in $(find . -type f -name "*.csproj"); do
                    grep -q netstandard1.3 $f
                    if [ $? -eq 0 ]; then
                        echo -e "\033[1mBuilding ${f} as netstandard1.3\033[0m"
                        dotnet build -c $c -o "${build_root}/${c}" \
                            --framework netstandard1.3 $f \
                            || return $?
                    else
                        grep -q netcoreapp1.1 $f
                        if [ $? -eq 0 ]; then
                            echo -e "\033[1mBuilding ${f} as netcoreapp1.1\033[0m"
                            dotnet build -c $c -o "${build_root}/${c}" \
                                --framework netcoreapp1.1 $f \
                                || return $?
                        fi
                    fi
                done
            done
        else
            echo "No dotnet installed, skipping dotnet build."
        fi
    fi
    return 0
}

# -----------------------------------------------------------------------------
# -- build in docker container
# -----------------------------------------------------------------------------
#container_build()
#{
# todo
#}

# -----------------------------------------------------------------------------
# -- Main
# -----------------------------------------------------------------------------
main()
{
    if [ -z "$build_configs" ]; then
        build_configs=(Debug Release)
    fi
    echo "Building ${build_configs[@]}..."
    if [ $build_clean == 1 ]; then
        echo "Cleaning previous build output..."
        rm -r -f "${build_root}"
    fi
    mkdir -p "${build_root}"
    native_build \
        || return $?
    managed_build \
        || return $?
}

process_args $*

set -e

main
echo "$?" 
cd "$current_root" > /dev/null
if [ $? -eq 0 ]; then
    echo "... Success!" 
else
    exit $?
fi



