
Reaper
======

Reaper is the first [OpenCL](http://en.wikipedia.org/wiki/Opencl) GPU  miner for
SolidCoin 2.0, coded by mtrlt, currently in an early stage of development. It is
open source and licensed under the GPL 3.0.

Reaper works well on both ATI and Nvidia GPUs. Although ATI GPUs appear to be
more efficient, many types of GPU remain untested. Users are encouraged to
experiment with the software and try to attain the best hash rate they can and
post their results on the official SolidCoin forum so they can be included in
the Mining Hardware Performance page.

As well as supporting GPU mining, Reaper can be configured to mine with the CPU
too. Enabling CPU mining does not normally have an impact on GPU mining hash
rates.

How to use
----------

Use from the command line, syntax:

    reaper <host> <port> <user> <pass> [config_filename]

Configuration file
------------------

The default configuration file name is **reaper.conf** which should be located
in the same directory as the executable.
The available configuration options are:

    cpu_mining_threads [number]

Used to specify on how many threads you want to mine on your CPU(s).

Recommended value: Number of logical cores. For example, if you have a quad core
CPU set this to 4. If you have a quad core with Hyper Threading, set it to 8.

    device [number]

Used to specify which GPU devices reaper should use. For example, if a user had
4 GPUs, inserting the following lines into the conf:

    device 0
    device 2

would make Reaper use devices 0 and 2, and leave devices 1 and 3 free. If there
are no device lines in the config, Reaper will attempt to use all available
GPUs.

    threads_per_device [number]

How many threads serve each GPU. Different types of GPU may benefit from a
higher or lower number, but 2 is optimal in most cases.

Recommended values: 1, 2 or 4

    aggression [number]/max

How much work is pushed onto the GPU at a time. Higher values for Aggression
typically produce higher hash rates. Experiment with different values to find
the best setting for your system.

From v10 onwards, there is a "maximum aggression" setting. It automatically sets
the aggression to an optimal value. It's useful for dedicated mining machines.
It is enabled like this: `aggression max` instead of a number.

Recommended values: max for dedicated miners, otherwise over 10

    worksize [number]

The size of the work sent to the GPU thread. Experiment with different values to
find the fastest hash rate for your setup. 128 seems to be optimal for most
setups.

Recommended values: 32, 64, 128, 256

    kernel [filename]

What file to use as the kernel. The default is reaper.cl.

Recommended value: reaper.cl

    save_binaries [yes/no]

Whether to save binaries after compiling. With this option enabled, subsequent
start-ups are faster. If this option is enabled, remember to delete the
binaries when updating drivers.

Recommended value: yes

    platform [number]

Select which GPU code platform to use. At the moment Reaper only supports
OpenCL, which is 0.

Recommended value: 0

    enable_graceful_shutdown [yes/no]

Whether to enable the "Graceful Shutdown" option. When this is enabled, users
can press "Q" then "Enter" to shut down Reaper gracefully.

Recommended value: yes

Compiling
---------

Reaper is compiled using [CMake](http://www.cmake.org/). In the reaper
directory, issue the following commands:

    mkdir build
    cd build
    cmake -D CMAKE_BUILD_TYPE=Release
    make

If you want to enable the experimental long polling support (only works with
pools that support it correctly) use this CMake command instead:

    cmake -D CMAKE_BUILD_TYPE=Release -D LONGPOLLING=ON

And if you want to disable compiling the OpenCL part, issue this command:

    cmake -D CMAKE_BUILD_TYPE=Release -D CPU_MINING_ONLY=ON

After compiling you can move the resulting `reaper` binary where you want, but
make sure to take along the `reaper.cl` kernel file as well as the `reaper.conf`
configuration file found in the root directory.

Known bugs
----------

    Kernel build not successful: -46

This bug seems to occur on systems with multiple GPUs. The best available
workaround is to try repeatedly until the error goes away.

    Windows reports MSVCP100.DLL missing

Install the [Microsoft Visual C++ 2010 Redistributable Package](http://www.microsoft.com/download/en/details.aspx?id=5555)

See Also
--------

[Mining Hardware Performance](http://wiki.solidcoin.info/wiki/Mining_Hardware_Performance)

