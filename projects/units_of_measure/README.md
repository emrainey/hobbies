# ISO Units of Measurement C++11 (or later) Library

To add a new unit modify the `units_of_measure.json` then run the script:

```bash
# move to the project directory
cd projects/units_of_measure

# Put them in a temporary directory for diffing
scripts/process_templates.py --json units_of_measure.json --outdir ../../build/generated

# Replace local files with the generated ones
scripts/process_templates.py --json units_of_measure.json --outdir .
```
