# MLIO Contrib Extensions

`contrib` is a package of utilities that work with MLIO but are not directly within the scope of the core MLIO library. Currently the `contrib` package includes:

* [`insights`](insights/README.md): a utility for analzing large datasets in a streaming manner using MLIO.


## Build Design

Each `contrib` module is built separately to the main MLIO Python extension and when enabled is copied in to the generated library as a sub-module that is reflected by the package name.
