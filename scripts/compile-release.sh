#!/bin/sh -x
ROOT=$1
mkdir cunit.dir
mkdir dynamic.dir
mkdir scheduler.dir
mkdir container.dir
mkdir data.dir
mkdir mycomponent.dir
mkdir mycomponent-static.dir
mkdir utest_data.dir
mkdir utest_json.dir
mkdir utest_scheduler.dir
mkdir utest_misc.dir
mkdir utest_cont.dir
mkdir utest_base64.dir
mkdir utest_logger.dir
mkdir runner.dir
mkdir utest_threadpool.dir
mkdir iot-static.dir
mkdir iot.dir
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/iot.c.o   -c $ROOT/src/c/iot.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/data.c.o   -c $ROOT/src/c/data.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/json.c.o   -c $ROOT/src/c/json.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/base64.c.o   -c $ROOT/src/c/base64.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/logger.c.o   -c $ROOT/src/c/logger.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/scheduler.c.o   -c $ROOT/src/c/scheduler.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/thread.c.o   -c $ROOT/src/c/thread.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/threadpool.c.o   -c $ROOT/src/c/threadpool.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/time.c.o   -c $ROOT/src/c/time.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/component.c.o   -c $ROOT/src/c/component.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/hash.c.o   -c $ROOT/src/c/hash.c
gcc  -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o iot-static.dir/container.c.o   -c $ROOT/src/c/container.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/iot.c.o   -c $ROOT/src/c/iot.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/data.c.o   -c $ROOT/src/c/data.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/json.c.o   -c $ROOT/src/c/json.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/base64.c.o   -c $ROOT/src/c/base64.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/logger.c.o   -c $ROOT/src/c/logger.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/scheduler.c.o   -c $ROOT/src/c/scheduler.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/thread.c.o   -c $ROOT/src/c/thread.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/threadpool.c.o   -c $ROOT/src/c/threadpool.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/time.c.o   -c $ROOT/src/c/time.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/component.c.o   -c $ROOT/src/c/component.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/hash.c.o   -c $ROOT/src/c/hash.c
gcc -Diot_EXPORTS -I$ROOT/src/c/../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o iot.dir/container.c.o   -c $ROOT/src/c/container.c
gcc  -I$ROOT/src/c/cunit/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o cunit.dir/Automated.c.o   -c $ROOT/src/c/cunit/Automated.c
gcc  -I$ROOT/src/c/cunit/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o cunit.dir/Basic.c.o   -c $ROOT/src/c/cunit/Basic.c
gcc  -I$ROOT/src/c/cunit/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o cunit.dir/CUError.c.o   -c $ROOT/src/c/cunit/CUError.c
gcc  -I$ROOT/src/c/cunit/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o cunit.dir/Console.c.o   -c $ROOT/src/c/cunit/Console.c
gcc  -I$ROOT/src/c/cunit/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o cunit.dir/TestDB.c.o   -c $ROOT/src/c/cunit/TestDB.c
gcc  -I$ROOT/src/c/cunit/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o cunit.dir/TestRun.c.o   -c $ROOT/src/c/cunit/TestRun.c
gcc  -I$ROOT/src/c/cunit/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o cunit.dir/Util.c.o   -c $ROOT/src/c/cunit/Util.c
gcc  -I$ROOT/src/c/examples/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o mycomponent-static.dir/mycomponent.c.o   -c $ROOT/src/c/examples/mycomponent.c
gcc -Dmycomponent_EXPORTS -I$ROOT/src/c/examples/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG -fPIC   -std=gnu11 -o mycomponent.dir/mycomponent.c.o   -c $ROOT/src/c/examples/mycomponent.c
gcc  -I$ROOT/src/c/examples/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o scheduler.dir/scheduler.c.o   -c $ROOT/src/c/examples/scheduler.c
gcc  -I$ROOT/src/c/examples/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o container.dir/container.c.o   -c $ROOT/src/c/examples/container.c
gcc  -I$ROOT/src/c/examples/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o container.dir/mycomponent.c.o   -c $ROOT/src/c/examples/mycomponent.c
gcc  -I$ROOT/src/c/examples/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o data.dir/data.c.o   -c $ROOT/src/c/examples/data.c
gcc  -I$ROOT/src/c/examples/../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o dynamic.dir/dynamic.c.o   -c $ROOT/src/c/examples/dynamic.c
gcc  -I$ROOT/src/c/utests/json/../../../../include -I$ROOT/src/c/utests/json/../../cunit   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o utest_json.dir/json.c.o   -c $ROOT/src/c/utests/json/json.c
gcc  -I$ROOT/src/c/utests/logger/../../../../include -I$ROOT/src/c/utests/logger/../../cunit   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o utest_logger.dir/logger.c.o   -c $ROOT/src/c/utests/logger/logger.c
gcc  -I$ROOT/src/c/utests/data/../../../../include -I$ROOT/src/c/utests/data/../../cunit   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o utest_data.dir/data.c.o   -c $ROOT/src/c/utests/data/data.c
gcc  -I$ROOT/src/c/utests/threadpool/../../../../include -I$ROOT/src/c/utests/threadpool/../../cunit   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o utest_threadpool.dir/threadpool.c.o   -c $ROOT/src/c/utests/threadpool/threadpool.c
gcc  -I$ROOT/src/c/utests/misc/../../../../include -I$ROOT/src/c/utests/misc/../../cunit   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o utest_misc.dir/misc.c.o   -c $ROOT/src/c/utests/misc/misc.c
gcc  -I$ROOT/src/c/utests/scheduler/. -I$ROOT/src/c/utests/scheduler/../../../../include -I$ROOT/src/c/utests/scheduler/../../cunit   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o utest_scheduler.dir/scheduler.c.o   -c $ROOT/src/c/utests/scheduler/scheduler.c
gcc  -I$ROOT/src/c/utests/base64/../../../../include -I$ROOT/src/c/utests/base64/../../cunit   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o utest_base64.dir/base64.c.o   -c $ROOT/src/c/utests/base64/base64.c
gcc  -I$ROOT/src/c/utests/runner/../../../../include   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o runner.dir/runner.c.o   -c $ROOT/src/c/utests/runner/runner.c
gcc  -I$ROOT/src/c/utests/container/../../../../include -I$ROOT/src/c/utests/container/../../cunit   -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_ -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG   -std=gnu11 -o utest_cont.dir/cont.c.o   -c $ROOT/src/c/utests/container/cont.c
ar qc libiot.a  iot-static.dir/iot.c.o iot-static.dir/data.c.o iot-static.dir/json.c.o iot-static.dir/base64.c.o iot-static.dir/logger.c.o iot-static.dir/scheduler.c.o iot-static.dir/thread.c.o iot-static.dir/threadpool.c.o iot-static.dir/time.c.o iot-static.dir/component.c.o iot-static.dir/hash.c.o iot-static.dir/container.c.o
ranlib libiot.a
gcc -fPIC  -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG  -shared -Wl,-soname,libiot.so -o libiot.so iot.dir/iot.c.o iot.dir/data.c.o iot.dir/json.c.o iot.dir/base64.c.o iot.dir/logger.c.o iot.dir/scheduler.c.o iot.dir/thread.c.o iot.dir/threadpool.c.o iot.dir/time.c.o iot.dir/component.c.o iot.dir/hash.c.o iot.dir/container.c.o -ldl -lrt 
ar qc libcunit.a  cunit.dir/Automated.c.o cunit.dir/Basic.c.o cunit.dir/CUError.c.o cunit.dir/Console.c.o cunit.dir/TestDB.c.o cunit.dir/TestRun.c.o cunit.dir/Util.c.o
ranlib libcunit.a
ar qc libmycomponent.a  mycomponent-static.dir/mycomponent.c.o
ranlib libmycomponent.a
gcc  -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG  -rdynamic scheduler.dir/scheduler.c.o  -o scheduler -Wl,-rpath,$ROOT/release/c libiot.so -ldl -lrt 
gcc -fPIC  -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG  -shared -Wl,-soname,libmycomponent.so -o libmycomponent.so mycomponent.dir/mycomponent.c.o -Wl,-rpath,$ROOT/release/c libiot.so -ldl -lrt 
gcc  -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG  -rdynamic container.dir/container.c.o container.dir/mycomponent.c.o  -o container libiot.a -ldl -lrt 
gcc  -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG  -rdynamic data.dir/data.c.o  -o data -Wl,-rpath,$ROOT/release/c libiot.so -ldl -lrt 
gcc  -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG  -rdynamic dynamic.dir/dynamic.c.o  -o dynamic -Wl,-rpath,$ROOT/release/c:$ROOT/release/c/examples libiot.so libmycomponent.so -ldl -lrt
ar qc libutest_json.a  utest_json.dir/json.c.o
ranlib libutest_json.a
ar qc libutest_logger.a  utest_logger.dir/logger.c.o
ranlib libutest_logger.a
ar qc libutest_threadpool.a  utest_threadpool.dir/threadpool.c.o
ranlib libutest_threadpool.a
ar qc libutest_data.a  utest_data.dir/data.c.o
ranlib libutest_data.a
ar qc libutest_misc.a  utest_misc.dir/misc.c.o
ranlib libutest_misc.a
ar qc libutest_scheduler.a  utest_scheduler.dir/scheduler.c.o
ranlib libutest_scheduler.a
ar qc libutest_base64.a  utest_base64.dir/base64.c.o
ranlib libutest_base64.a
gcc  -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration -pthread -pipe -D_GNU_SOURCE -D__NO_STRING_INLINES -DIOT_BUILD_COMPONENTS -DIOT_BUILD_DYNAMIC_LOAD -O3 -DNDEBUG  -rdynamic runner.dir/runner.c.o  -o runner -Wl,-rpath,$ROOT/release/c libcunit.a libutest_json.a libutest_cont.a libutest_logger.a libutest_data.a libutest_threadpool.a libutest_misc.a libutest_scheduler.a libutest_base64.a libiot.a libiot.so -ldl -lrt 
ar qc libutest_cont.a  utest_cont.dir/cont.c.o
ranlib libutest_cont.a
