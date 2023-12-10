# (Input-Queued) Switch Scheduling Simulator

# Installation 

## Linux
Start by cloning from the parent repository using. 
``` 
git clone https://github.gatech.edu/matpadikar3/ssched_simulator.git
```
Now's a good time to install needed dependancies. Let's start by installing vcpkg. For more information, head to the [vcpkg website.](https://vcpkg.io/en/getting-started)
```
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
```

Before we install vcpkg libraries, ensure that our system has the needed linux packages. 
```
sudo apt install pkg-config
sudo apt install libfmt-dev
sudo apt install zlib1g-dev
sudo apt install lemon
```

With Vcpkg up and running, we look at the various vcpkg libraries needed. Namely, 
```
cd vcpkg
./vcpkg install fmt
./vcpkg install lemon
./vcpkg install spdlog
./vcpkg install nlohmann-json
./vcpkg install catch2
```

After installing all the vcpkg libraries, Run Cmake. Ensure to replace [Path] with your installation folder. 
If you run into errors at this stage, have a look at [Troubleshooting](#troubleshooting)
```
cd ../ssched-simulator/build
cmake -B [Path]/ssched_simulator/build -S . -DCMAKE_TOOLCHAIN_FILE=/[Path]/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Before proceeding, delete the ``` CMakeCache.txt ``` file under ```ssched_simulator/build```. 



## Running the code

This only serves as an example to run one experiment. More experiments can be found under [Experiments](experiments)
```
make
cd build
./ssched_simulator ../experiments/delay_port.json
```


## Troubleshooting
### 1)Catch.hpp not found error
Replace ```catch.hpp``` with ```catch_all.hpp``` wherever the error occurs. 

### 2)Unable to locate package libfmt-dev
As per [This](https://askubuntu.com/questions/1205765/c-fmt-library-installation-is-not-working) thread on askubuntu
```
sudo add-apt-repository universe
sudo apt update
sudo apt install libfmt-dev
```
### 3) Vcpkg was unable to detect the active compiler's information
This error usually occurs when gcc and g++ are not correctly set up.
```
sudo apt install gcc
sudo apt install g++
```
### 4) Cannot execute Binary file: Exec format error
This problem is specific to WSL 2.0 running Ubuntu. Disable systemd in wsl.conf file and restart. 
Change directory to /etc
```
sudo nano wsl.conf
systemd=false
```
Restart your wsl instance. 

# Specific README

## Scheduler README
**Refer the specific README [src/scheduler/README.md](src/scheduler/README.md)**

## Batch Scheduling
**Refer the specific README [src/scheduler/batch_scheduler/README.md](src/scheduler/batch_scheduler/README.md)**

## Algorithms under Exploration

### SB-QPS Basic
A packet scheduling algorithm for network systems that operates in discrete time slots, managing queues associated with output ports. It often follows a round-robin or FIFO approach, and serves as a foundational reference for network simulations and research experiments.

### Oblivious Half & Half SB-QPS

Oblivious Half & Half SB-QPS is an extension of the SB-QPS packet scheduling algorithm. This variant operates in discrete time slots, managing queues associated with output ports. In each time slot, it employs a unique strategy: the first half adheres to a normal SB-QPS approach, while the second half allows longer proposals to attempt previous time slots within the same frame without checking availability. This approach aims to optimize packet scheduling for specific scenarios, making it valuable for network simulations and research experiments.

### Availability-Aware Half & Half SB-QPS: 

Availability-Aware Half & Half SB-QPS is a specialized variant of the SB-QPS packet scheduling algorithm. It operates in discrete time slots, managing queues associated with output ports. In each time slot, it combines two distinct strategies: the first half follows a normal SB-QPS approach, while the second half allows proposals to attempt previous time slots while considering availability. This unique approach enhances scheduling efficiency by taking into account resource availability

### Adaptive SB-QPS

Adapative SB-QPS is a simple extension to the [A-A H&H SB-QPS](#availability-aware-half--half-sb-qps). The first half follows the normal SB-QPS Approach, while the second half allows trying earlier time slots, as long as there are available ones. 

### Multi-Iter Versions

This refers to variants that invole multiple iterations or passes when making packet scheduling decisions with a single time slot. Multi-Iter versions are designed to optimize various aspects of packet scheduling, such as improving throughput, fairness, or Quality of Service (QoS). 

### Multi-Accept Multi-Iter Versions

Multi-Accept Multi-Iter algorithms are a much more complex evolution of the SB-QPS algorithm. It allows multiple packets from the same queue to be selected during each iteration and potentially during multiple iterations within a single time slot. 

# Co-Authors

## Dhruv Rauthan
[dhruvrauthan@gatech.edu](dhruvrauthan@gatech.edu)

## Akshat Deo
[akshatdeo@gatech.edu](akshatdeo@gatech.edu)

# Acknowledgments

## Professor Jun Xu, Georgia Institute of Technology
[https://jimxuosu.github.io/](https://jimxuosu.github.io/)

## Jingfan Meng
[https://sites.cc.gatech.edu/grads/j/jmeng40/](https://sites.cc.gatech.edu/grads/j/jmeng40/)

## Long Gong
[https://lgong30.github.io/](https://lgong30.github.io/)
