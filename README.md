# OpenNN Machine Learning for y-- (Ymm)

OpenNN deep learning and neural network library bindings for y--.

## Installation

This library is an official external module for the **y--** (**Ymm**) programming language. You can install it effortlessly via **`yenv`**:

```bash
yenv install opennn
```

This will automatically clone the repository, build the shared library (`liby-opennn.so` / `y-opennn.dll`), and register it in `~/.yenv/lib/`.

## Usage

```ymm
import OpenNN

define function main(): {
    # Neural network modeling and machine learning
    print("OpenNN module loaded")
}
```

## Documentation

Full documentation, including manual pages and reference texts, are available in the [`docs/`](docs/) directory:
- `opennn.man`: UNIX troff man page
- `opennn.txt`: Plaintext documentation reference

You can view the documentation using:
```bash
man -l docs/opennn.man
```

## License

This project is licensed under the same license as the **y--** programming language. See the [LICENSE](LICENSE) file for details.
