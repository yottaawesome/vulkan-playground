# gsl

Re-exports [Microsoft GSL](https://github.com/microsoft/GSL) pointer and string vocabulary types as a module
partition of `vulkangfx`.

## Dependency rules

This is a **ground-level** partition. It must not depend on any other `vulkangfx`
partitions. Other partitions may depend on `:gsl`.

## Partitions

| Partition | Purpose                                       |
|-----------|-----------------------------------------------|
| `:gsl`    | Re-exports `gsl::not_null`, `gsl::czstring` |
