#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

current_root=$(pwd)
repo_root=$(cd "$(dirname "$0")/.." && pwd)

skip_unittests=OFF
skip_dotnet=0
use_zlog=OFF
MAKE_PARALLEL_JOBS=1

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
    echo " -c --clean                     Build clean (Removes previous build output)"
    echo " -C --config <value>            [Debug] Build configuration (e.g. Debug, Release)"
    echo " -o --build-root <value>        [/build] Directory in which to place all files during build"
    echo "    --use-zlog                  Use zlog as logging framework instead of xlogging"
    echo "    --skip-unittests            Skips building and executing unit tests"
    echo "    --skip-dotnet               Do not build dotnet core API and packages"
    echo "    --pack-only                 Only creates packages. (Cannot be combined with --clean)"
    echo " -n --nuget-folder <value>      [/build] Folder to use when outputting nuget packages."
    echo " -x --xtrace                    print a trace of each command"
    echo " -j <value> | --jobs <value>    Number of parallel make jobs, see description of make -j"
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
    TEMP=`getopt -o xo:C:cn:j: \
          -l xtrace,build-root:,config:,clean,use-zlog,use-openssl,use-libwebsockets,pack-only,skip-unittests,skip-dotnet,os:,nuget-folder:,jobs: \
         -- "$@"`

    if [ $? != 0 ]
    then
        echo "Failed to parse options"
        usage
        exit 1
    fi

    # Note the quotes around `$TEMP': they are essential!
    eval set -- "$TEMP"

    while true
    do
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
	    -c | --clean)
		build_clean=1
                shift ;;
	    --use-zlog)
		use_zlog=ON
                shift ;;
	    --use-openssl)
                shift ;;
	    --use-libwebsockets)
                shift ;;
	    --pack-only)
		build_pack_only=1
                shift ;;
	    --skip-unittests)
		skip_unittests=ON
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
	    --) shift
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

			cmake -DCMAKE_BUILD_TYPE=$c -Dskip_unittests:BOOL=$skip_unittests \
			      -Duse_zlog:BOOL=$use_zlog "$repo_root" \
                              -DLWS_IPV6:BOOL=ON || \
			    return 1
                        # Start as much parallel jobs as requested by the user.
                        # Until the load average equals the number of cores.
                        # Be verbose if something goes wrong
			make -j$MAKE_PARALLEL_JOBS --load-average=`nproc` || make VERBOSE=1 || \
				return 1
			if [ $skip_unittests == ON ]; then
				echo "Skipping unittests..."
			else
				#use doctored (-DPURIFY no-asm) openssl
				export LD_LIBRARY_PATH=/usr/local/ssl/lib
				ctest -C "$c" --output-on-failure || \
					return 1
				export LD_LIBRARY_PATH=
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

				for f in $(find . -type f -name "project.json"); do
					grep -q netstandard1.3 $f
					if [ $? -eq 0 ]; then
						if [ $build_pack_only == 0 ]; then
							echo -e "\033[1mBuilding ${f} as netstandard1.3\033[0m"
							dotnet build -c $c -o "${build_root}/${c}" \
								--framework netstandard1.3 $f \
								|| return $?
						fi
						dotnet pack --no-build -c $c $f \
							-o "${build_nuget_output}/${c}" \
							|| return $?
					elif [ $build_pack_only == 0 ]; then
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



