# MLIO Insights

The insights contrib package provides tools to analyze large tabular datasets using MLIO.
In particular, it is well suited to be combined with MLIO's support for large CSV datasets.

All functionality in the MLIO insights package uses streaming and approximate algorithms to
ensure that memory usage is bound. Similarly, as a CPU bound task it uses the same 
multithreading primitives as MLIO to achieve better performance.

## Usage

```py
import mlio
from mlio.contrib.insights import analyze_dataset

rdr_prm = mlio.DataReaderParams(dataset=mlio.list_files(csv_files),
                                batch_size=batch_size)
                                
csv_prm = mlio.CsvParams(default_data_type=mlio.DataType.STRING,
                         header_row_index=None,
                         max_line_length=100_000,
                         allow_quoted_new_lines=True)
                         
reader = mlio.CsvReader(rdr_prm, csv_prm)

null_values = set(['null', 'none', 'nil', 'na', 'nan'])
columns_to_capture = set([1, 5, 9])
max_capture_amount = 1000

results = analyze_dataset(reader, 
                          null_values,
                          columns_to_capture,
                          max_capture_amount)

# Not necessary unless you want to use the reader again.
reader.reset()

# analyze_dataset returns a DataAnalysis which has
# the list of columns and relevant information.
for column_info in results.columns:
    # Fields can be accessed directly:
    print(column_info.string_captured_unique_values)

    # Or the entire set of column_info can be returned as a dictionary.
    print(column_info.to_dict())
```

Each column in this Example will print a dictionary like the following:

```
{
    'rows_seen': '109435',
    'numeric_count': '109435',
    'numeric_finite_count': '109435',
    'numeric_nan_count': '0',
    'numeric_int_count': '5760',
    'string_empty_count': '0',
    'string_min_length': '1',
    'string_min_length_not_empty': '1',
    'string_max_length': '3',
    'string_avg_length': '2.250000',
    'string_only_whitespace_count': '0',
    'string_null_like_count': '0',
    'numeric_finite_mean': '5.863193',
    'numeric_finite_min': '4.600000',
    'numeric_finite_max': '7.700000',
    'numeric_finite_median_approx': '6.0',
    'example_value': '5.1',
    'string_cardinality': 16,
    `string_vocab_cardinality`: 16,
    'string_captured_unique_values': {'6.5': 5760, '6.4': 5760, '5.7': 5760, '6.1': 5760, '5': 5760, '5.6': 11520, '6.7': 5760, '4.6': 5760, '5.9': 5760, '7.7': 11520, '6.2': 5760, '5.8': 11520, '5.4': 5760, '4.7': 5760, '4.9': 5760, '5.1': 5755},
    'string_captured_unique_values_overflowed': False
}
```

## Available Information

The following information on each column is available:

**General Information**

- `rows_seen`: number of rows (including empty/null).
- `name`: name of the column or it's index as a string.

**Numeric Analysis**

- `numeric_count`: the count of values that could be parsed as a float/double.
- `numeric_nan_count`: the number of values that could not be parsed as a float/double.
- `numeric_finite_count`: the number of non-infinity/non-NaN values.
- `numeric_int_count`: the number of values that could be parsed as an integer.
- `numeric_finite_mean`: the average of finite (non-infinite) numeric values seen.
- `numeric_finite_min`: the minimum finite numeric value seen.
- `numeric_finite_max`: the maximum finite numeric value seen.
- `numeric_finite_median_approx`: the approximate median of up to a sample of 10000 finite numeric values seen

**String Analysis**

- `string_cardinality`: an approximation for the cardinality for values as strings (e.g. `1.0` and `1` are considered different).
- `string_vocab_cardinality`: an approximation for the number of unique words seen across all values (delimited by space).
- `string_min_length`: the minimum length of a string encountered in this column.
- `string_min_length_not_empty`: the minimum length of a string encountered in this column that was not empty.
- `string_max_length`: the minimum length of a string encountered in this column.
- `string_avg_length`: the average length of a string across the entire column.
- `string_empty_count`: the number of values without any characters.
- `string_only_whitespace_count`: the number of values with only whitespace characters.
- `string_null_like_count`: the number of values that match the `null_like_values` option in a case-insensitive manner.


**Captured Values**
- `example_value`: a value of the column sampled from the dataset.
- `string_captured_unique_values`: if the column was specified in `capture_columns`; the first `max_capture_count` unique string values and frequencies will be saved.
- `string_captured_unique_values_overflowed`: if the column has more unique string values than `max_capture_count` this will be set to true.
