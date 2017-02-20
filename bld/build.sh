#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

set -e

repo_root=$(cd "$(dirname "$0")/.." && pwd)

skip_unittests=OFF
skip_dotnet=0
use_zlog=OFF

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
	echo "    --os <value>			   [] Os to build on (needs Docker installed)."
    echo " -c --clean                  Build clean (Removes previous build output)"
    echo " -C --config <value>         [Debug] Build configuration (e.g. Debug, Release)"
    echo " -o --build-root <value>     [/build] Directory in which to place all files during build"
    echo "    --use-zlog               Use zlog as logging framework instead of xlogging"
    echo " 	  --skip-unittests         Skips building and executing unit tests"
	echo "	  --skip-dotnet            Do not build dotnet core API and packages"
	echo "    --pack-only              Only creates packages. (Cannot be combined with --clean)"
	echo " -n --nuget-folder <value>   [/build] Folder to use when outputting nuget packages."
    echo " -x --xtrace                 print a trace of each command"
    exit 1
}

# -----------------------------------------------------------------------------
# -- Parse arguments
# -----------------------------------------------------------------------------
process_args ()
{
    save_next_arg=0
    for arg in $*; do
		  if [ $save_next_arg == 1 ]; then
			build_root="$arg"
			save_next_arg=0
		elif [ $save_next_arg == 2 ]; then
			build_configs+=("$arg")
			save_next_arg=0
		elif [ $save_next_arg == 3 ]; then
			build_nuget_output="$arg"
			save_next_arg=0
		elif [ $save_next_arg == 4 ]; then
			build_os=="$arg"
			save_next_arg=0
		else
			case "$arg" in
				-x | --xtrace)
					set -x;;
				-o | --build-root)
					save_next_arg=1;;
				-C | --config)
					save_next_arg=2;;
				-c | --clean)
					build_clean=1;;
				--use-zlog)
					use_zlog=ON;;
				--use-openssl)
					;;
				--use-libwebsockets)
					;;
				--pack-only)
					build_pack_only=1;;
				--skip-unittests)
					skip_unittests=ON;;
				--skip-dotnet)
					skip_dotnet=1;;
				--os)
					save_next_arg=4;;
				-n | --nuget-folder)
					save_next_arg=3;;
				*)
					usage;;
			esac
		fi
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
			pushd "${build_root}/cmake/${c}" > /dev/null
			cmake -DCMAKE_BUILD_TYPE=$c -Dskip_unittests:BOOL=$skip_unittests \
					-Duse_zlog:BOOL=$use_zlog "$repo_root" || \
				return 1 
                        # Use all available CPU cores for make in silent mode. But be verbose if something goes wrong
			make -j`nproc` || make VERBOSE=1 || \
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
			popd > /dev/null
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
			pushd "${repo_root}/api/csharp" > /dev/null
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
			popd > /dev/null
		else
			echo "No dotnet installed, skipping dotnet build."
		fi
	fi
	return 0
}

# -----------------------------------------------------------------------------
# -- build in container
# -----------------------------------------------------------------------------
#container_build()
#{
# todo
#}

pushd "$repo_root" > /dev/null
process_args $*
if [ -z "$build_configs" ]; then
	build_configs=(Debug Release)
fi
echo "Building ${build_configs[@]}..."
if [ $build_clean == 1 ]; then
    echo "Cleaning previous build output..."
    rm -r -f "${build_root}"
fi
mkdir -p "${build_root}"
native_build || exit 1
managed_build || exit 1
popd > /dev/null

[ $? -eq 0 ] || exit $?

