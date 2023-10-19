#!/bin/bash
#set -o pipefail
#

set -e

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/../.." && pwd)
run_unit_tests=ON
make_install=
run_valgrind=0
run_unittests=OFF

usage ()
{
    echo "build.sh [options]"
    echo "options"
    echo " -cl, --compileoption <value>  specify a compile option to be passed to gcc"
    echo "   Example: -cl -O1 -cl ..."
    echo "-rv, --run_valgrind will execute ctest with valgrind"
    echo "--run-unittests run the unit tests"

    exit 1
}

process_args ()
{
    save_next_arg=0
    extracloptions=" "

    for arg in $*
    do
      if [ $save_next_arg == 1 ]
      then
        # save arg to pass to gcc
        extracloptions="$arg $extracloptions"
        save_next_arg=0
      elif [ $save_next_arg == 2 ]
	  then
        build_root="$arg"
        save_next_arg=0
      else
          case "$arg" in
              "-cl" | "--compileoption" ) save_next_arg=1;;
              "-i" | "--install" ) make_install=1;;
              "-rv" | "--run_valgrind" ) run_valgrind=1;;
              "--run-unittests" ) run_unittests=ON;;
              "--build-root" ) save_next_arg=2;;
              * ) usage;;
          esac
      fi
    done
}

process_args $*
log_dir=$build_root
build_folder=$build_root"/cmake/shared-util_linux"

# Set the default cores
CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)

rm -r -f $build_folder
mkdir -p $build_folder
pushd $build_folder
cmake -DcompileOption_C:STRING="$extracloptions" -Drun_valgrind:BOOL=$run_valgrind $build_root -Drun_unittests:BOOL=$run_unittests
make --jobs=$CORES
if [[ $make_install == 1 ]] ;
then
    echo "Installing packaging" 
    # install the package
    make install
fi

if [[ $run_valgrind == 1 ]] ;
then
	#use doctored openssl
	export LD_LIBRARY_PATH=/usr/local/ssl/lib
	ctest -j $CORES --output-on-failure
	export LD_LIBRARY_PATH=
else
	ctest -j $CORES -C "Debug" --output-on-failure
fi

popd
