# ZMatter

A basic [Matter](https://github.com/project-chip/connectedhomeip) on/off light
bulb running on [Zephyr](https://github.com/zephyrproject-rtos/zephyr)'s
`native_sim` board, so it runs as a regular Linux process.

It uses only upstream, Apache-2.0 licensed components:

| Component       | Version                                     |
| --------------- | ------------------------------------------- |
| Zephyr          | v4.4.2                                      |
| connectedhomeip | `master` @ `da40d15d` (pinned in `west.yml`) |
| Mbed TLS / TF-PSA-Crypto | as shipped with Zephyr v4.4.2      |

The device exposes:

- endpoint 0: Root Node
- endpoint 1: On/Off Light (Identify, Groups, On/Off, Descriptor)

The data model is defined in [app/light.zap](app/light.zap) (derived from the
upstream chef `rootnode_onofflight` device). Changes of the On/Off attribute
are reported in the log (`Light is ON` / `Light is OFF`).

## How it works on native_sim

On `native_sim`, Matter talks to the network through the host's BSD sockets
(this is upstream's default for `CONFIG_ARCH_POSIX`), so the Zephyr IP stack
is disabled and no TAP interface or root privileges are needed. Commissioning
and control happen over the host's own interfaces, the same way as for the
Linux example apps.

This needs a small patch to connectedhomeip, kept in
[zephyr/patches](zephyr/patches) and applied with `west patch`:

- the Zephyr `net_if` helpers are only built when Matter uses the Zephyr
  `net_if` API (glibc and Zephyr networking headers cannot be mixed), and
- the event loop polls the host sockets instead of blocking in `select()`,
  which would otherwise stall the simulated CPU and its clock.

## Getting started

Before getting started, make sure you have a proper Zephyr development
environment. Follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).
`native_sim/native/64` builds with the host GCC; no Zephyr SDK toolchain is
needed.

### Initialization

```shell
west init -m https://github.com/walidbadar/ZMatter --mr main my-workspace
cd my-workspace
west update
west patch apply
```

### Matter build tools

The Matter library is built with GN and its data model is generated with ZAP.
Both are Apache-2.0 licensed and can be installed into the workspace:

```shell
CHIP_ROOT=modules/lib/connectedhomeip
mkdir -p .tools/gn .tools/zap

curl -sSL -o gn.zip "https://chrome-infra-packages.appspot.com/dl/gn/gn/linux-amd64/+/latest"
unzip -q gn.zip -d .tools/gn

ZAP_VERSION=$(cat $CHIP_ROOT/scripts/setup/zap.version)
curl -sSL -o zap.zip "https://github.com/project-chip/zap/releases/download/${ZAP_VERSION}/zap-linux-x64.zip"
unzip -q zap.zip -d .tools/zap

pip install -r $CHIP_ROOT/scripts/setup/requirements.build.txt \
    -c $CHIP_ROOT/scripts/setup/constraints.txt

export PATH=$PWD/.tools/gn:$PATH
export ZAP_INSTALL_PATH=$PWD/.tools/zap
```

### Building and running

```shell
west build -b native_sim/native/64 ZMatter/app
./build/zephyr/zephyr.exe
```

On boot the device prints its onboarding codes. With the default test
configuration these are:

- setup PIN code: `20202021`, discriminator: `3840`
- manual pairing code: `34970112332`
- QR code: `MT:Y.K90AFN00KA0648G00`

The Matter state (fabrics, attributes) is stored in the simulated flash,
`flash.bin`, in the current directory. Delete it to factory reset the device.

A debug configuration is also provided:

```shell
west build -b native_sim/native/64 ZMatter/app -- -DEXTRA_CONF_FILE=debug.conf
```

### Controlling the light

Commission and control the device with
[chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)
running on the same host:

```shell
chip-tool pairing onnetwork 1 20202021
chip-tool onoff toggle 1 1
chip-tool onoff read on-off 1 1
```

> [!WARNING]
> The device uses the test Device Attestation Certificate and test setup
> codes from connectedhomeip. It is intended for development only.
