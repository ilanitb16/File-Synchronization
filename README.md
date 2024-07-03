# File Synchronization
Synchronizing file access, implement a synchronization lock for file access , a buffered I/O library in C, implement a C library that provides functionalities similar to Python's shutil.copytree.

## Part1 Output:
- ```cd part1```
- ```gcc -o part1 part1.c```
- ``` ./part1 "Parent message" "Child1 message" "Child2 message" 3 ```
- ![image](https://github.com/ilanitb16/File-Synchronization/assets/97344492/3bfd71e9-bf08-47ef-89b5-cc5298e2a199)


## Part2 Output:
- ```cd Part2```
- ```gcc part2.c -o part2```
- example of input: ```./part2 "First message" "Second message" "Third message" 3 > output2.txt```
- ![image](https://github.com/ilanitb16/File-Synchronization/assets/97344492/dd1f3e1e-aecf-495b-88e4-55dc727819f8)

## Part3 Output:
- ```cd Part3```
-  Run the project / run the tests by extracting them from the tests folder (and adding the files to ```Part3```, then executing ```./script.sh```

All tests completed: 

![image](https://github.com/ilanitb16/File-Synchronization/assets/97344492/47faf584-b6b5-4777-a1eb-5c6e7a4b7b0d)

### Part4 Output:

- ```cd Part4```
- Execute the following commands:
  ```
  gcc -c copytree.c -o copytree.o
  ar rcs libcopytree.a copytree.o
  gcc part4.c -L. -lcopytree -o main_program
  ./main_program source_directory destination_directory
  ```
- Test with sybmolic link and permissions: ``` ./main_program -l -p source_directory destination_directory ```
- These are the permissions Iv'e given the files in the folder ` source_directory`:

-
   ```
  # Create the directory structure
  mkdir -p source_directory/subdir1
  mkdir -p source_directory/subdir2

  # Create the files
  touch source_directory/file1.txt
  touch source_directory/file2.txt
  touch source_directory/subdir1/file3.txt
  touch source_directory/subdir1/file4.txt
  touch source_directory/subdir2/file5.txt
  
  # Set the permissions
  chmod 644 source_directory/file1.txt    # rw-r--r--
  chmod 600 source_directory/file2.txt    # rw-------
  chmod 755 source_directory/subdir1      # rwxr-xr-x
  chmod 640 source_directory/subdir1/file3.txt  # rw-r-----
  chmod 600 source_directory/subdir1/file4.txt  # rw-------
  chmod 777 source_directory/subdir2      # rwxrwxrwx
  chmod 444 source_directory/subdir2/file5.txt  # r--r--r--
  
  # Create the symbolic link
  ln -s ../file1.txt source_directory/subdir2/link_to_file1
  ```
- You can test any other permissions or use these.

![WhatsApp Image 2024-06-29 at 15 03 30_ac60bd09](https://github.com/ilanitb16/File-Synchronization/assets/97344492/50f1e906-264b-4bda-8bb8-96285b417e67)


![WhatsApp Image 2024-06-29 at 15 06 29_e0509a0f](https://github.com/ilanitb16/File-Synchronization/assets/97344492/05136979-0345-4d24-bf0d-54b7e190f3fe)
![WhatsApp Image 2024-06-29 at 15 09 37_3895b8a8](https://github.com/ilanitb16/File-Synchronization/assets/97344492/c9ad9c2a-218a-400f-add0-db4e5a0ea693)
![WhatsApp Image 2024-06-29 at 15 09 41_604c6174](https://github.com/ilanitb16/File-Synchronization/assets/97344492/84efde65-33c5-4b59-87f1-3fb8e81d9533)






