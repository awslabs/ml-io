# ML-IO Insights

The insights contrib package provides tools to analyze large tabular datasets using ML-IO.
In particular, it is well suited to be combined with ML-IO's support for large CSV datasets.

All functionality in the ML-IO insights package uses streaming and approximate algorithms to
ensure that memory usage is bound. Similarly, as a CPU bound task it uses the same 
multithreading primitives as ML-IO to achieve better performance.

## Usage

```py
import mlio
from mlio.contrib.insights import analyze_dataset

reader = mlio.CsvReader(dataset=mlio.list_files(csv_files),
                        batch_size=batch_size,
                        default_data_type=mlio.DataType.STRING,
                        header_row_index=None,
                        max_line_length=100_000,
                        allow_quoted_new_lines=True)

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

Each column in this example will print a dictionary like the following:

```
{
    'rows_seen': '2305812', 
    'numeric_count': '2305761', 
    'numeric_nan_count': '51', 
    'string_empty_count': '0', 
    'string_min_length': '2', 
    'string_min_length_not_empty': '2', 
    'string_max_length': '2', 
    'string_only_whitespace_count': '0', 
    'string_null_like_count': '0', 
    'numeric_mean': '40.936210', 
    'numeric_min': '18.000000', 
    'numeric_max': '95.000000', 
    'example_value': '', 
    'string_cardinality': 78, 
    'string_captured_unique_values': {'70', '38', '80', '30', '84', '82', '42', '53', '73', '75', '95', 'V1', '35', '74', '64', '36', '39', '60', '61', '44', '33', '85', '90', '66', '41', '56', '67', '79', '22', '54', '46', '37', '51', '81', '62', '65', '93', '28', '24', '88', '68', '57', '59', '21', '71', '72', '76', '78', '34', '29', '19', '48', '23', '94', '83', '92', '87', '58', '18', '32', '45', '63', '69', '86', '27', '89', '20', '55', '77', '31', '47', '26', '49', '43', '52', '50', '40', '25'}, 
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
- `numeric_finite_mean`: the average of finite (non-infinite) numeric values seen.
- `numeric_finite_min`: the minimum finite numeric value seen.
- `numeric_finite_max`: the maximum finite numeric value seen.

**String Analysis**

- `string_cardinality`: an approximation for the cardinality for values as strings (e.g. `1.0` and `1` are considered different).
- `string_min_length`: the minimum length of a string encountered in this column.
- `string_min_length_not_empty`: the minimum length of a string encountered in this column that was not empty.
- `string_max_length`: the minimum length of a string encountered in this column.
- `string_empty_count`: the number of values without any characters.
- `string_only_whitespace_count`: the number of values with only whitespace characters.
- `string_null_like_count`: the number of values that match the `null_like_values` option in a case-insensitive manner.

**Captured Values**
- `example_value`: a value of the column sampled from the dataset.
- `string_captured_unique_values`: if the column was specified in `capture_columns`; the first `max_capture_count` unique string values will be saved.
- `string_captured_unique_values_overflowed`: if the column has more unique string values than `max_capture_count` this will be set to true.