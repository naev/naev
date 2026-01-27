
# Plugin Generator: a Tutorial

`plugin_generator` allows you to automatically generate a plugin based on
your Naev repository.

### Step 0: have a Naev repository !

``` bash
git clone https://codeberg.org/naev/naev.git my_naev_repo
cd my_naev_repo
```

### Step 1: modify the contents of `dat/` as you see fit

In this example, we replace the Za'lek-Reiben-Regas line with just
Zal'ek-Regas, and remove Regas spobs.

Commit your changes:
```
[main 87a7d9e927] my commit

Date: Wed Jan 21 18:51:29 2026 +0100
7 files changed, 2 insertions(+), 119 deletions(-)
delete mode 100644 dat/spob/arcadia.xml
delete mode 100644 dat/spob/gastan.xml
delete mode 100644 dat/spob/seiben_iv.xml
delete mode 100644 dat/spob/seiben_xi.xml
delete mode 100644 dat/ssys/seiben.xml
```

In particular, we have changed:
``` diff
diff --git a/dat/ssys/regas.xml b/dat/ssys/regas.xml
index 9fee16cfcf..5fc636514a 100644
--- a/dat/ssys/regas.xml
+++ b/dat/ssys/regas.xml
@@ -18,4 +18,4 @@
-  <jump target="Seiben">
+  <jump target="Za'lek">
    <autopos/>
    <hide>0.000000</hide>
   </jump>
diff --git a/dat/ssys/zalek.xml b/dat/ssys/zalek.xml
index d1142a1933..de5e322bb3 100644
--- a/dat/ssys/zalek.xml
+++ b/dat/ssys/zalek.xml
@@ -38,4 +38,4 @@
-  <jump target="Seiben">
+  <jump target="Regas">
    <autopos/>
    <hide>0.000000</hide>
   </jump>
```

### Step 2: Call `gen_plugin`

``` bash
 > ./utils/gen_plugin.sh "My Plugin" --author Myself
Your plugin either adds/removes files or includes modifications
in dat/ on non-xml files -> it won't be considered as mainline-safe.
  adding: dat/ (stored 0%)
  adding: dat/ssys/ (stored 0%)
  adding: dat/ssys/regas.xml (deflated 54%)
  adding: dat/ssys/zalek.xml (deflated 64%)
  adding: plugin.toml (deflated 42%)
my_plugin.zip
```

Notice that overwrite confirmation will be asked if there is already a
directory with the same name, while the zip file will be overwritten
mercilessly.

You can check the plugin parameters in `my_plugin/plugin.toml`:
``` toml
identifier = "MyPlugin"
name = "My Plugin"
author = "plugin generator"
version = "1.0.0"
abstract = "plugin generated from repo status"
description = "(mainline-UNSAFE)"
license = "GPLv3+"
release_status = "development"
tags = [ ]
naev_version = ">= 0.14.0-alpha.1"
source = "local"
blacklist = [dat/spob/arcadia.xml, dat/spob/gastan.xml, dat/spob/seiben_iv.xml, dat/spob/seiben_xi.xml, dat/ssys/seiben.xml]
```

### Need more information ?

``` bash
 > ./utils/gen_plugin.sh --help
```
