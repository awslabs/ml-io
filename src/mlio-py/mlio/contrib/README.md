# ML-IO Contrib Extensions

`contrib` is a package of utilities that work with ML-IO but are not directly within the scope of the core ML-IO library. Currently the `contrib` package includes:

* `insights`: a utility for analzing large datasets in a streaming manner using ML-IO.


## Build Design

Each `contrib` module is built separately to the main ML-IO Python extension and when enabled is copied in to the generated library as a sub-module that is reflected by the package name.
