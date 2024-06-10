#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
# include <linux/fs.h>

//	typical structure:
//		offset 0000(0): 45520200
//		offset c000(49152): 4c4b			=> part to mount (sdh1, loop, etc)
//7FFFFE00 (2147483136)
//8000BE00 (2147532288)


// Block size of the underlying device
// Top-level partitions cannot start at a different boundary
// const size_t BLKSIZE = 512; // flash is 512

bool is_apple_partition(const std::vector<uint8_t>& data, size_t index)
{
    // 4552 => Apple Partition
    // 0200 => 512 bytes sector

    return std::memcmp(&data[index], "\x45\x52\x02\x00", 4) == 0;
}

// Returns device size in bytes
uint64_t get_device_size(const std::string& path)
{
	int fd = open(path.c_str(), O_RDONLY);
	if (fd == -1) {
		std::cerr << "No such file [" << path << "]\n";
		return (uint64_t)-1;
	}

	uint64_t cap = 0;

	if (ioctl(fd, BLKGETSIZE64, &cap) == -1) {
		perror("ioctl");
		std::cerr << "Cannot get device size [" << path << "]\n";
		close(fd);
		return (uint64_t)-1;
	}

	close(fd);

	return cap;
}

int main(int argc, char* argv[])
{
	std::vector<std::string> args(argv, argv + argc);
	std::string device_path = args[1];
	size_t device_size = get_device_size(device_path);

	if (device_size == (uint64_t)-1)
	{
		return EXIT_FAILURE;
	}

	printf("* Device is %lu bytes\n", device_size);

	int fd = open(device_path.c_str(), O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "Failed to open [%s]\n", device_path.c_str());
		return EXIT_FAILURE;
	}

	printf("* Scanning for apple partitions...\n");

	// Load the file in 'fd' in 1 megabtyes chunks
	size_t chunk_size = 1024 * 1024; // 1 megabyte
	std::vector<uint8_t> chunk(chunk_size);
	size_t bytes_read = 0;
	while (bytes_read < device_size)
	{
		size_t new_bytes_read = 0;
		size_t remaining_bytes = device_size - bytes_read;
		size_t read_size = std::min(chunk_size, remaining_bytes);
		// lseek(fd, bytes_read, SEEK_SET);
		ssize_t result = read(fd, chunk.data(), read_size);
		if (result == -1)
		{
			fprintf(stderr, "Failed to read device file\n");
			close(fd);
			return EXIT_FAILURE;
		}
		new_bytes_read += result;

		// Look for an apple partition every 512 bytes
		for (size_t i = 0; i < result; i += 512)
		{
			// printf( "[%lx %02X%02X%02X%02X]\n", bytes_read + i, chunk[i], chunk[i + 1], chunk[i + 2], chunk[i + 3]);
			if (is_apple_partition(chunk, i))
			{
				uint64_t partition_start = bytes_read + i;
				printf("** Found Apple Partition Map at offset %lx (%lu)\n", partition_start, partition_start);
				uint16_t block_size = (chunk[i + 2] << 8) | chunk[i + 3];
				uint32_t block_count = (chunk[i + 4] << 24) | (chunk[i + 5] << 16) | (chunk[i + 6] << 8) | chunk[i + 7];
				printf("**  Blocksize: %x (%u)\n", block_size, block_size);
				printf("**  Blockcount: %x (%u)\n", block_count, block_count);
				uint64_t offset = partition_start + block_count * block_size;

				// jump to the end of the partition
				// printf("** Next partition at offset %lx (%lu)\n", offset, offset);
				new_bytes_read = offset - (offset % chunk_size);    // round to chuck size
				// printf("** Rescanning from offset %lx (%lu)\n", new_bytes_read, new_bytes_read);
				lseek(fd, new_bytes_read, SEEK_SET);

				// Prints the losetup command:
				printf( "\n# To mount as IMG:\n" );
				printf("DEVICE=`sudo losetup --find --show --offset %lu --sizelimit %lu %s`\n", partition_start + 0xc000, block_count * (uint64_t)block_size - 0xc000, device_path.c_str());
				printf( "rm  ~/AAA-Mac/temp\n" );
				printf( "ln -s $DEVICE ~/AAA-Mac/temp\n" );
				printf( "sudo chmod o+rw $DEVICE\n" );
				printf( "\n# To mount as hda:\n" );
				printf("DEVICE=`sudo losetup --find --show --offset %lu --sizelimit %lu %s`\n", partition_start, block_count * (uint64_t)block_size, device_path.c_str());
				printf( "rm  ~/AAA-Mac/temp\n" );
				printf( "ln -s $DEVICE ~/AAA-Mac/temp\n" );
				printf( "sudo chmod o+rw $DEVICE\n" );
				printf( "\n# To unmount:\n" );
				printf( "sudo losetup -d $DEVICE\n\n" );
			}
		}

		bytes_read = new_bytes_read;
	}

	printf("done\n");

	close(fd);

	return 0;
}
