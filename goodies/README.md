# Goodies

This forder contains useful stuff.

## `italyprog.zip`

This is an enhanced keyboard map for italian keyboards that adds two shortcuts for characters that are often sed in programming and on linux: tilda `~` and backtick `.

Those characters are mapped to `AltGR` + `'` and `AltGr` + `ì`, respectively.

**NOTE: this is only needed if you have a physical italian keyboard**

To install it, proceed as follows:

1. unzip
2. run the installer `setup.exe`
3. Go to Settings > Time & language > Language & region, then click on the dots beside your language (Italian), then under the section "keyboard" click on "Add a keyboard" and search for "Italian - Programmer keys"
4. optionally, remove the original italian keymap

## MQTT
### Running the broker

The MQTT broker can be launched using the Docker container `eclipse-mosquitto`. Note that it must be launched from the PowerShell console and **not** from the WSL console, for the latter runs a virtual machine with its own network that is not reachable from the outside. So, open the PowerShell in this folder and type:

```PowerShell
docker run -it --rm -p1883:1883 -v"${PWD}/mosquitto.conf:/mosquitto/config/mosquitto.conf" eclipse-mosquitto
```

Note that that the `mosquitto.conf` file must be in the current directory.

### Installing `libmosquitto` on Linux

We are going to build a client for the MQTT broker. To do so, we are using a well-known library named `libmosquitto`. The library could be installed with the Ubuntu package manager as `sudo apt install libmosquitto-dev`. Unfortunately, this command installs a pretty old version of the library, version 1.6.9 at the time of writing, while the current version is 2.0.14.

Consequently, we are going to download the source of the latest version and compile it. Proceed as follows, in a WSL terminal:

```sh
cd ~
wget https://mosquitto.org/files/source/mosquitto-2.0.14.tar.gz
tar xzf mosquitto-2.0.14.tar.gz
cd mosquitto-2.0.14
cmake -Bbuild . -DCMAKE_BUILD_TYPE=Release -DWITH_TLS=OFF -DDOCUMENTATION=OFF -DWITH_LIB_CPP=OFF -DWITH_SRV=OFF -DWITH_CJSON=OFF
cmake --build build
sudo cmake --install build
```

Now you can verify that the installation went fine with the command:

```sh
mosquitto_sub --help
```

If you get an error with loading shared libraries (and only in this case!) do the following:

```sh
cat /etc/ld.so.conf.d/libc.conf
```

If the output **does not contain** the path `/usr/local/lib`, then type:

```sh
echo "/usr/local/lib" | sudo tee -a /etc/ld.so.conf.d/libc.conf
```

Then (and in any case, even if the previous command **does report** `/usr/local/lib`) type:

```sh
sudo ldconfig
```

Now it should work.

### Installing `libmosquitto` on MacOS

On MacOS, libmosquitto can be easily installed with the command:

```sh
brew install mosquitto
```
