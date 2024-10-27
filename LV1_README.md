# LV1

## Copying files and looking at buffer size and performance correlation

Move to the following directory:
```
[Repo]/x64/
```

Create files using the following commands:
```
fsutil file createnew 1mb_file 1048576
fsutil file createnew 1gb_file 1073741824
fsutil file createnew 5gb_file 5368709120
```

Open CMD and use the following commands
```
[EXE_NAME].exe [ORIGIN_FILE] [DESTINATION_FILE]
```