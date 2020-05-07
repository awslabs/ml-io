# Miscellaneous
* [Classes](#S3Client)
  * [S3Client](#S3Client)
* [Functions](#Functions)
    * [initialize_aws_sdk](#initialize_aws_sdk)
    * [deallocate_aws_sdk](#dispose_aws_sdk)

This page describes a list of types and functions that do not fall under a specific category.

## S3Client
Represents a client to access Amazon S3. If no `access_key_id` and `secret_key` are provided, uses the [default AWS credential lookup rules](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html).

```python
S3Client(access_key_id : str = None,
         secret_key : str = None,
         session_token : str = None,
         profile : str = None,
         region : str = None,
         use_https : bool = True)
```

- `access_key_id`: The access key ID to use.
- `secret_key`: The secret key to use.
- `session_token`: The session token to use.
- `profile`: The profile name to use.
- `region`: The region to use. If not specified, defaults to us-east-1.
- `use_https`: A boolean value indicating whether to use HTTPS for communication.

## Functions
#### initialize_aws_sdk
Initializes AWS C++ SDK. If you are using MLIO along with another library or framework that initializes AWS C++ SDK, you might/should skip calling this function; otherwise, this function has to be called before instantiating an [S3Client](#S3Client).

```python
initialize_aws_sdk()
```

#### deallocate_aws_sdk
Deallocates the internal data structures used by AWS C++ SDK. It is recommended to call this function when you no longer use an [S3Client](#S3Client).

```python
deallocate_aws_sdk()
```
