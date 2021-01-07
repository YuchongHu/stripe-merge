StripeMerge
=====

Here is the source code of `StripeMerge` prototype system described in our paper submitted to ICDCS'21. 

Preparation
----

We implement `StripeMerge` on Ubuntu-16.04. So the tutorial as follows is also based on Ubuntu-16.04.

Users can use `apt` to install the required libraries.

 -  g++
 - make & cmake
 - nasm
 - libtool & autoconf
 - git

```bash
$  sudo apt install g++ make cmake nasm autoconf libtool git
```

`StripeMerge`  is built on ISA-L and Sockpp. Users can install these two libraries manually:

- **Intel®-storage-acceleration-library (ISA-L)**.

  ```bash
  $  git clone https://github.com/intel/isa-l.git
  $  cd isa-l
  $  ./autogen.sh
  $  ./configure; make; sudo make install
  ```

- **Sockpp**

  ```bash
  $  git clone https://github.com/fpagliughi/sockpp.git
  $  cd sockpp
  $  mkdir build ; cd build
  $  cmake ..
  $  make
  $  sudo make install
  $  sudo ldconfig
  ```


## Installation

- Users can install  `StripeMerge` via make.

  ```bash
  $  cd stripe-merge
  $  make all
  ```

## Configuration

- Before running, users should prepare the `nodes_ip.text`, in which are IPs of all `helper_nodes`. 

- Next, prepare the configure. The example configure are presented in `nodes_config.ini`, where the first line is RS code parameter `k` and  `m`, followed by  the IPs of helper_nodes in need.

- You can also generate the `nodes_config.ini` using script `util/cloud_exp.py`, by passing RS code parameter `k`, `m` as well as the number of `helper_nodes` that actually runs. 

  ```bash
  $  python3 util/cloud_exp.py get_config  k  m  nodes_num
  ```

- Users can use  script `util/cloud_exp.py` to update the configure or program in `helper_nodes` either.  (To utilize this tool, user should configure that the `master_node` connect to all `helper_nodes` by SSH without password. Note that the script is based on the assumption that the `helper_nodes` are mapped orderly like node01/node02/.../node32 in `/etc/hosts`.)

  ```bash
  $  python3 util/cloud_exp.py update [all/config/program] nodes_num
  ```

- Use `dd` to generate a random block in all `helper_nodes` to test.

  ```bash
  $  dd if=/dev/urandom of=test/stdfile bs=1M count=64
  ```

## Run it

- To run the **prototype system**, users should run the `master_node` (at the root directory of repository),

  ```bash
  $  cd stripe-merge
  $  ./build/bin/node_main 0 stripes_size [exp_type: 0-p, 1-g, 2-ncs]
  ```

  and run the `helper_nodes` respectively (at the root directory of repository, too).

  ```bash
  $  cd stripe-merge
  $  ./build/bin/node_main [1/2/3/...]
  ```

  Or Users can use the script on  `master_node`  to conduct the experiment. (Same requirement  as above mentioned.)

  ```bash
  $  python3 util/cloud_exp.py exp  stripes_size  k  m  nodes_num  [0-p, 1-g, 2-ncs]
  ```

- You can also run the **matching simulator** locally with the parameter you want, to compare `StripeMerge-P` with`StripeMerge-G` .

  ```bash
  $  ./build/bin/matching_main [stripes_num] [host_num] [rs_k] [rs_m]
  ```

  