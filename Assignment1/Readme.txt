Some Assumptions:

1). If the server closes, Client prints a message on standard output "Server has stopped responding.\n" and then terminates, as there is no server to connect to.

2). If the Server-IP-Address is not valid, Client prints a message on standard output "no such HOST", and similar general error messages if programs fails at vulnerable places.

3). Buffer Capacity is 4096 bytes.

4). Filenames are expected to be without Whitespaces and Non-Empty(as they should be).

5). If the File is empty, Client will be prompted for another File.
