# Dependencies
```shell
sudo apt install libudev-dev pkg-config
git clone git@github.com:libusb/hidapi.git
git checkout hidapi-0.14.0
cmake -S <source_dir> -B <build_dir> # optional: -DCMAKE_INSTALL_PREFIX='/custom/install/path' 
sudo cmake --build <buid_dir> --target install # installs to /usr/local/ by default 
```

create file: /etc/udev/rules.d/99-footpedal.rules
```shell
SUBSYSTEM=="usb", ATTR{idVendor}=="1a86", ATTR{idProduct}=="e026", MODE="0666", GROUP="plugdev"  
# if necessary you can add more devices by repeating this for each device use lsusb to determine 
# your pedals id
```

Note, you can use FootPedal::DetermineExpectedPedalValue() to figure out what value is send on 
depression of the pedal# footpedal_hid_device
