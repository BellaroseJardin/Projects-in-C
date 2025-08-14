
# Tiny C Password Manager (XOR-encrypted, portable)

A minimal, portable, terminal-based password manager written in C.  
It stores credentials (service, username, password) in a single **encrypted binary file** using a simple XOR keystream derived from a master password and a random salt.

> ⚠️ **Security note:** This is an educational/demo project. XOR-based crypto is *not* strong encryption. For real-world use, switch to a vetted library (e.g., OpenSSL AES-GCM). Employers will still appreciate the architecture, cleanliness, and portability.

---

## Features
- Add, list, find, and delete credentials
- Single **master password** to unlock your vault
- Salted key derivation and XOR keystream encryption
- Clean modular C code: `main.c`, `db.c/h`, `crypto.c/h`, `ui.c/h`
- Simple **Makefile** and no external dependencies
- Works on Linux/macOS/WSL; Windows (MinGW) should also work
- No memory leaks (checked with Valgrind)

## Build
```bash
make
# Binary will be at ./build/pwdmgr
```

## Run
```bash
./build/pwdmgr [-f path/to/vault.dat]
```
If the specified vault file doesn't exist, you'll be prompted to create one and set a master password.

## Usage
Interactive TUI menu with options:
1. Add credential
2. List credentials
3. Find credential by service
4. Delete credential by service
5. Save vault
6. Change master password
7. Exit

### Example
```
$ ./build/pwdmgr
Enter path to vault (default: vault.dat): 
Vault not found. Create new vault? (y/n): y
Set master password: ********
[+] Vault created: vault.dat

== Tiny Password Manager ==
1) Add credential
2) List credentials
3) Find credential by service
4) Delete credential by service
5) Save vault
6) Change master password
7) Exit
> 
```

## File Format (for reviewers)
- Header (unencrypted):
  - magic: `PWDDB1\0`
  - version: `1`
  - salt: random 32-bit
  - count: number of credentials
- Payload (encrypted with XOR keystream):
  - `count` credentials; each is a fixed-size struct of ASCII/UTF-8 C strings

## Testing
Basic crypto test:
```bash
make test
./build/tests
```

## Valgrind (optional)
```bash
valgrind --leak-check=full ./build/pwdmgr
```

## License
MIT
