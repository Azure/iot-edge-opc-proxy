@echo off
rem 
rem install docker and pass an optional linux flavor
rem

pushd docker
docker build -t proxy:build %1\.
docker run --cpu-count 1 -ti proxy:%1
popd