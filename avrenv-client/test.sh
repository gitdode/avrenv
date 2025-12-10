#!/bin/bash

set -eu
set -o pipefail

workdir=$(realpath $(dirname "$0"))
srvdir="$workdir/../avrenv-web"
command="nohup ./mvnw quarkus:dev"
url="http://localhost:8080/data"

cd "$srvdir"

nohup $command &
pid=$!

echo "Waiting for service to be up..."

time=0
while ([[ $(curl -o /dev/null -s -w "%{http_code}\n" $url) != 200 ]]); do
    sleep 1;
    ((time=$time+1))
    if [[ $time -gt 60 ]]; then
        echo "Timeout waiting for service to be up"
        kill $pid
        exit 1
    fi
done

cd "$workdir"

make test; test=$?

kill $pid

exit $test

