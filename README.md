## Overview
 This tool hides text inside a image using LSB steganography. It works by encoding secret data in the least significant bits of the image pixels, and you can use it to hide a message or password while keeping the image visually the same. The encoder embeds your secret text and the decoder extracts it. This makes it useful for sending passwords more safely, because instead of sending the password as plain text, you place it inside an image, and only someone with the decoder tool can read the hidden message.
or this
## How to install
#### Install the required package
```bash
sudo apt-get update
sudo apt-get install -y build-essential pkg-config libpng-dev
```
#### Build the Project
```bash
make        # build the binaries 
make clean  # remove generated binaries
```
## Usage
#### Encode a message
```bash
./lsb-encode -i <input.png> -o <output.png> -m <message>
```
#### Decode a message
```bash
./lsb-decode -i <input.png>
```
## License
This project is licensed under the MIT License. See [LICENSE](https://github.com/whiterunx64/lsb-stego-png/blob/main/LICENSE) for details.
