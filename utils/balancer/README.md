## Naev Balancer

Balancer is a script that is useful for reading/writing outfit and ship data
to/from csv files. This allows for editing using spreadsheet applications such
as openoffice.


### Usage

```
usage: balancer.py [-h] {outfit,ship} {w,r} filename
balancer.py: error: the following arguments are required: mode, readwrite, filename
```

Common usage case would be the following:
1. Read from the XML data using `readwrite=r`, e.g., `./utils/balancer/balancer.py ship r ship.csv`.
2. Edit the resulting `ship.csv` with open office.
   * It is possible to remove unnecessary columns excluding the `name` column
   * Empty cells will be ignored, new values will be added
3. Import the changes to the XML data using `readwrite=w`, e.g., `./utils/balancer/balancer.py ship w ship.csv`


### Notes

1. Balancer can automatically find and deal with new tags. Furthermore, you can create new columns to create new tags, or just edit a single XML file to have it get detected.
2. Currently, balancer doesn't deal with attributes such as `<tag attribute='value'>...</tag>`. These must be manually edited.
