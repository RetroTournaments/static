MiSTER FPGA - For Static
========================


Shopping list
-------------

Note: some of these may be affiliate links.

- [DE10 Nano dev kit](https://www.mouser.com/ProductDetail/Terasic-Technologies/P0496?qs=%2FacZuiyY%252B4ZdDLJqTxdJ5w%3D%3D).
- [MiSTer FPGA IO Analog Pro](https://misteraddons.com/products/mister-fpga-io-analog-pro)
- [MiSTer FPGA SDRAM](https://misteraddons.com/products/sdram?_pos=2) or maybe [Amazon Link](https://amzn.to/4itWIRP)



- [64 GB Micro SD](https://amzn.to/4isMPnt) (or equivalent)
- [Micro USB Hub](https://amzn.to/41RDMW3) (or equivalent)
- [USB Keyboard](https://amzn.to/4kOxO0H) (or equivalent)
- Appropriate length Ethernet cables / connectivity or [Wifi Dongle](https://amzn.to/4itkFIY) (or equivalent)


For Serial Connection,
User_IO[4] is now out, as TXD -> RX+
User_IO[6] is now in, as RXD -> TX+

- [SNAC Adapter w/ exposed pins](https://misterfpga.co.uk/product/mister-snac-adapter-usb/) w/ single NES Adapter (or equivalent)
- [Serial Converter Adapter](https://amzn.to/4c126te)
- Necessary things like flux, pins, cables, etc


Setup
-----

- Ensure switches on DE10 Nano: small dip: 101011, big dip: 1111
- Download [Mr Fusion v2.9 iso](https://github.com/MiSTer-devel/mr-fusion/releases)
- Using [Balena Etcher](https://etcher.balena.io/#download-etcher) or equivalent flash the Mr Fusion ISO to the 64 GB SD Card.
- Install SDRAM into DE10 Nano
- Install IO Analog Pro onto DE10 Nano
- Power it, connect to network / tv / keyboard / etc.
- Obtain the MiSTer's ip address (esc -> left arrow on mister, or `scripts/find_mister.sh` from computer)

- Run the setup script with the ip address `scripts/setup_mister.sh 192.168.1.xx`





