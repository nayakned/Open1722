# ACF-CAN Applications

The two applications available in this folder are acf-can-listener and acf-can-talker. These applications can be used along with Linux CAN utilities. On Ubuntu/Debian Linux distributions, these utilities can be installed using the package manager `apt install can-utils`

## acf-can-talker
_acf-can-talker_ receives frames on a (virtual) CAN interface and send out the corresponding IEEE 1722 ACF messages. This application also supports a UDP encapsulation for the IEEE 1722 messages. The parameters for its usage are as follows:

```
Usage: acf-can-talker [OPTION...]

acf-can-talker -- a program designed to send CAN messages to a remote CAN bus
over Ethernet using Open1722.

      --canif                CAN interface
  -c, --count=COUNT          Set count of CAN messages per Ethernet frame
  -d, --dst-addr=MACADDR     Stream destination MAC address (If Ethernet)
      --fd                   Use CAN-FD
  -i, --ifname               Network interface (If Ethernet)
  -n, --dst-nw-addr          Stream destination network address and port (If
                             UDP)
  -t, --tscf                 Use TSCF
  -u, --udp                  Use UDP
  -?, --help                 Give this help list
      --usage                Give a short usage message
```

## acf-can-talker
_acf-can-listener_ receives IEEE 1722 ACF messages and puts out the corresponding CAN frames on a (virtual) CAN interface. Analogous to the _acf_can_talker_, UDP encapsulation is also available for this application.  The parameters for its usage are as follows:

```
Usage: acf-can-listener [OPTION...]

acf-can-listener -- a program designed to receive CAN messages from a remote
CAN bus over Ethernet using Open1722.

      --canif                CAN interface
  -d, --dst-addr=MACADDR     Stream destination MAC address (If Ethernet)
      --fd                   Use CAN-FD
  -i, --ifname               Network interface (If Ethernet)
  -p, --udp-port             UDP Port to listen on (if UDP)
  -u, --udp                  Use UDP
  -?, --help                 Give this help list
      --usage                Give a short usage message

```

## Quickstart Tutorial: Tunneling CAN over IEEE 1722 using Linux CAN utilities
Here is an example of how CAN frames can be tunneled over an Ethernet link using _acf-can-talker_ and _acf-can-listener_.
We use two virtual CAN interfaces, _vcan0_ and _vcan1_, here which can be setup using following commands:
```
$ ip link add dev vcan0 type vcan   # Execute these commands also for vcan1
$ ip link set dev vcan0 up
```

### Generate CAN traffic
On Terminal 1, generate CAN traffic for vcan0:
```
$ cangen vcan0
```

In the following, we tunnel this generated CAN traffic over Ethernet to _vcan1_

### Use Talker Application for tunneling
On Terminal 2, pipe generated CAN traffic from vcan0 to _acf-can-talker_. Here, we use UDP encapsulation.
```
$ ./acf-can-talker -u --dst-nw-addr 127.0.0.1:17220 --canif vcan0
```
Alternatively, we can directly use Ethernet for transport.
```
$ ./acf-can-talker --dst-addr aa:bb:cc:dd:ee:ff -i eth0 --canif vcan0
```

### Use Listener Application for receiving
On Terminal 3, receive the IEEE 1722 traffic using _acf-can-listener_ for putting the CAN frame out on vcan1.

If the talke uses UDP encapsulation:
```
$ acf-can-listener -u -p 17220 --canif vcan1
```

Alternatively, if Ethernet is directly used:
```
$ ./acf-can-listener --dst-addr aa:bb:cc:dd:ee:ff -i eth0 --canif vcan0
```

You can now compare CAN traffic seen on _vcan0 and vcan1_, if the tunneling has worked.
Note that the tunneling works in these examples only in one direction (_vcan0_ -> _vcan1_).


