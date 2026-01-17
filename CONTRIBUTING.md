# Contributing to Hack WiFi AI

Thank you for your interest in contributing to Hack WiFi AI! We appreciate your time and effort to help improve this project.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#-getting-started)
- [Development Workflow](#-development-workflow)
- [Code Style](#-code-style)
- [Testing](#-testing)
- [Pull Request Process](#-pull-request-process)
- [Reporting Issues](#-reporting-issues)
- [Feature Requests](#-feature-requests)
- [Documentation](#-documentation)

## Code of Conduct

This project and everyone participating in it is governed by our [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## 🚀 Getting Started

1. **Fork** the repository on GitHub
2. **Clone** your fork locally:
   ```bash
   git clone https://github.com/HoffmannAlex/WiFi-Password-Hacker-FREE.git
   cd hack-wifi-ai-cpp
   git remote add upstream https://github.com/HoffmannAlex/WiFi-Password-Hacker-FREE.git
   ```
3. **Create a branch** for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```
4. **Set up the development environment** (see [README.md](README.md) for details)

## 🔄 Development Workflow

1. **Sync** with the main repository:
   ```bash
   git fetch upstream
   git merge upstream/main
   ```

2. **Make your changes** following the code style guidelines

3. **Test your changes** thoroughly

4. **Commit your changes** with a descriptive message:
   ```bash
   git commit -m "Add: New feature description"
   ```

5. **Push** to your fork:
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Open a Pull Request** from your fork to the main repository

## 🎨 Code Style

- Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use 4 spaces for indentation (no tabs)
- Maximum line length: 100 characters
- Use `PascalCase` for class names and `camelCase` for variables and functions
- Include Doxygen-style comments for all public interfaces
- Keep functions small and focused on a single responsibility

### Formatting

We use `clang-format` for consistent code formatting. Before committing, run:

```bash
find src/ include/ -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i -style=file
```

## 🧪 Testing

### Running Tests

```bash
# Run all tests
ctest --output-on-failure

# Run specific test
./tests/hack_wifi_ai_tests --gtest_filter=TestSuite.TestName
```

### Writing Tests
- Add tests for new features
- Follow the existing test structure
- Test both success and failure cases
- Keep tests independent and isolated

## 🔄 Pull Request Process

1. Ensure your code passes all tests
2. Update documentation as needed
3. Ensure your branch is up to date with the latest changes from `main`
4. Open a pull request with a clear title and description
5. Reference any related issues
6. Wait for code review and address any feedback

## 🐛 Reporting Issues

When reporting issues, please include:

1. A clear, descriptive title
2. Steps to reproduce the issue
3. Expected vs. actual behavior
4. Environment details (OS, compiler version, etc.)
5. Any relevant error messages or logs

## 💡 Feature Requests

We welcome feature requests! Please:

1. Check if a similar feature already exists
2. Explain why this feature would be valuable
3. Include any relevant use cases

## 📚 Documentation

- Keep documentation up to date with code changes
- Add or update comments when modifying code
- Update the README for significant changes
- Add examples for new features

## 🤝 Code Review Process

1. A maintainer will review your pull request
2. Address any feedback or requested changes
3. Once approved, your changes will be merged

## 📜 License

By contributing, you agree that your contributions will be licensed under the project's [LICENSE](LICENSE).

---

Thank you for your contribution! Your work helps make Hack WiFi AI better for everyone. 🎉
