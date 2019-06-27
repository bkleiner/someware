package main

import (
	"fmt"
	"io"
	"log"

	"github.com/jacobsa/go-serial/serial"
)

func main() {
	options := serial.OpenOptions{
		PortName:        "/dev/ttyACM0",
		BaudRate:        19200,
		DataBits:        8,
		StopBits:        1,
		MinimumReadSize: 4,
	}

	// Open the port.
	port, err := serial.Open(options)
	if err != nil {
		log.Fatalf("serial.Open: %v", err)
	}

	// Make sure to close it later.
	defer port.Close()

	// Write 4 bytes to the port.
	if _, err := port.Write([]byte{'T'}); err != nil {
		log.Fatalf("port.Write: %v", err)
	}

	buf := make([]byte, 64)
	for {
		n, err := port.Read(buf)
		fmt.Print(string(buf[:n]))

		if err == io.EOF {
			break
		}
	}
}
