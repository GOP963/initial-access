# initial-access

## Embedding External Files into Executables: PyInstaller vs. C/C++

### 1. PyInstaller – Embedding Files into an EXE

PyInstaller provides a mechanism for bundling external files, such as PDF documents, images, configuration files, and other resources, into a standalone executable.

The `--add-data` argument is used to include external files in the application bundle.

Example:

```
pyinstaller --onefile --noconsole --icon=pdf.ico --add-data "FakeResume.pdf;." --name resume resume.py
```

Argument explanation:

| Argument         | Description                                             |
| ---------------- | ------------------------------------------------------- |
| `--onefile`      | Packages the application into a single executable.      |
| `--noconsole`    | Prevents the console window from appearing.             |
| `--icon=pdf.ico` | Sets the executable's application icon.                 |
| `--add-data`     | Includes external data files in the application bundle. |
| `--name`         | Specifies the output executable name.                   |

How does it work internally?

When PyInstaller uses the `--onefile` option, it creates a self-extracting executable.

At runtime:

1. The bootloader initializes.
2. The embedded application archive is accessed.
3. Required files are extracted into a temporary directory.
4. The Python application is initialized.
5. The application can access bundled files through the runtime extraction directory.

PyInstaller exposes this temporary directory through:

```
sys._MEIPASS
```

This is why applications can locate bundled files using `sys._MEIPASS` instead of relying on the executable's original directory.

Important distinction: PyInstaller's `--add-data` does not simply add files to the Windows PE Resource Directory. It uses its own application packaging and archive mechanism.

### 2. Windows C/C++ – Embedding Files Using PE Resources

In native Windows applications written in C or C++, a similar capability is available through the Windows Resource system.

The Windows Resource Compiler (`rc.exe`) allows developers to embed external files into executable files during compilation.

Common resource types include:

| Resource Type | Description           |
| ------------- | --------------------- |
| `ICON`        | Application icons     |
| `BITMAP`      | Bitmap images         |
| `STRINGTABLE` | String resources      |
| `DIALOG`      | Dialog templates      |
| `RCDATA`      | Arbitrary binary data |

For example, `RCDATA` can represent binary assets such as PDF documents, images, or configuration files.

Unlike PyInstaller's application archive, native Windows resources are stored in the PE Resource Directory.

### 3. Windows Resource API

Windows provides several APIs for accessing embedded resources:

| API              | Purpose                                 |
| ---------------- | --------------------------------------- |
| `FindResource`   | Locates a resource in the executable.   |
| `LoadResource`   | Loads the resource.                     |
| `LockResource`   | Returns a pointer to the resource data. |
| `SizeofResource` | Retrieves the resource size.            |

These APIs allow native applications to access embedded binary resources at runtime.

### 4. PyInstaller vs. Native C/C++

| Feature              | PyInstaller         | Native C/C++          |
| -------------------- | ------------------- | --------------------- |
| File embedding       | `--add-data`        | Resource Compiler     |
| Icon configuration   | `--icon`            | ICON Resource         |
| Binary resources     | Application archive | `RCDATA`              |
| Runtime access       | `sys._MEIPASS`      | Windows Resource API  |
| Packaging mechanism  | PyInstaller archive | PE Resource Directory |
| Temporary extraction | Bootloader-managed  | Application-managed   |

### 5. Important PE Structure

In native Windows executables, resources are represented through the PE Resource Directory.

A typical PE section layout looks like this:

```
PE File
│
├── DOS Header
├── NT Headers
├── Section Table
│
├── .text
├── .rdata
├── .data
├── .rsrc
└── .reloc
```

The `.rsrc` section commonly contains:

- Icon resources
- Version information
- Manifest resources
- RCDATA resources

You can inspect these structures using PE-bear, CFF Explorer, or Resource Hacker.

Key takeaway:

PyInstaller and native Windows C/C++ applications both support embedding external files inside executable packages. However, they use different mechanisms: PyInstaller uses its own application archive and bootloader, whereas native Windows applications can use the PE Resource Directory and Windows Resource API.


<img width="897" height="320" alt="image" src="https://github.com/user-attachments/assets/fbbe0daa-f510-4c33-9223-d6fddb184808" />

<img width="1014" height="375" alt="image" src="https://github.com/user-attachments/assets/dfd32e1f-47fd-4df0-ac28-147f31680c02" />



## Reference
https://www.youtube.com/watch?v=ygrVBsBw31Y
https://github.com/ASTRA-LabsHQ/Malware-Development/blob/main/PDF-ReverseShell/resume.py

