# Hermes Modules

##

```sh
hermesbutler.py <board id> enable -l <link> --dis 
hermesbutler.py <board id> udp-config -l <link> <rx endpoint> <tx endpoint>
hermesbutler.py <board id> mux-config -l <link> 1 2 3
hermesbutler.py <board id> fakesrc-config -l <link> -n 4
hermesbutler.py <board id> enable -l <link> --en
hermesbutler.py <board id> stats -l <link>
```

## Hermes IPbus UDP-to-Axi bridge

### Installation

Installing the ipbus bridge requires compiling the bridge code on the zynq PS and running the bridge from terminal or as a service. `gcc`

Log in onto the Zynq PS. Clone the `hermesmodules` via git or by downloading a tarball.

```sh
git clone https://github.com/DUNE-DAQ/hermesmodules.git -b <tag or branch>
```
or
```sh
curl -o <path to tarball on GitHub>
tar xvgz <tarball>
```

Compile the bridge code with make

```sh
cd zynq
make
```
*Optional*: Install `hermes_udp_srv` system-wide 

* On the ZCU102

    ```sh
    sudo make install-zcu102
    ```

* On the WIBs
    ```sh
    sudo make install-zcu102
    ```

## Running `hermes_udp_srv`

When installed system-wide, the `hermes_udp_srv` is started automatically at boot.
If not, the service can be started from commandline

```
Usage: ./hermes_udp_srv [options...]
Options:
    -d, --device           device type             (Required)
    -v, --verbose          verbosity level        
    -c, --check-replies-countCheck Replies count    
    -h, --help             Shows this page        
```


* On the ZCU102
    ```sh
    sudo /bin/hermes_udp_srv -d zcu102 
    ```

* On the WIBs
    ```sh
    sudo /bin/hermes_udp_srv -d wib -c false 
    ```


