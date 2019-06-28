package main

import (
	"bufio"
	"encoding/binary"
	"encoding/json"
	"log"
	"net/http"
	"strconv"
	"strings"

	svg "github.com/ajstarks/svgo"
	"github.com/jacobsa/go-serial/serial"
)

type Message struct {
	Dataset string
	Payload map[string]string
}

const (
	StartState = iota
	PayloadLabelState
	PayloadDataState
)

func parse(s string) Message {
	m := Message{
		Payload: make(map[string]string),
	}
	label, state := "", StartState
	start, pos, size := 0, 0, len(s)

	for pos < size {
		switch state {
		case StartState:
			if s[pos] == ' ' {
				m.Dataset = strings.TrimSpace(s[start:pos])
				state = PayloadLabelState
				start = pos
			}
			pos++
		case PayloadLabelState:
			if s[pos] == ':' {
				label = strings.TrimSpace(s[start:pos])
				state = PayloadDataState

				// eat the following space
				if pos+1 < size && s[pos+1] == ' ' {
					pos++
				}

				start = pos
			}
			pos++
		case PayloadDataState:
			if pos == size-1 || s[pos] == ',' || s[pos] == ' ' {
				if pos == size-1 {
					pos++
				}

				m.Payload[label] = strings.TrimSpace(s[start:pos])
				state = PayloadLabelState

				// eat the following space
				if pos+1 < size && s[pos+1] == ' ' {
					pos++
				}

				start = pos
			}
			pos++
		}
	}

	return m
}

func mustParseFloat(s string) float32 {
	if s == "" {
		return 0
	}

	v, err := strconv.ParseFloat(s, 32)
	if err != nil {
		log.Fatal(err)
	}
	return float32(v)
}

type Vector struct {
	Roll  float32
	Pitch float32
	Yaw   float32
}

var datasets = map[string][]Vector{}

func readSerial() {
	options := serial.OpenOptions{
		PortName:        "/dev/ttyACM0",
		BaudRate:        19200,
		DataBits:        8,
		StopBits:        1,
		MinimumReadSize: 4,
	}

	port, err := serial.Open(options)
	if err != nil {
		log.Fatalf("serial.Open: %v", err)
	}
	defer port.Close()

	/*
		if _, err := port.Write([]byte{'T'}); err != nil {
			log.Fatalf("port.Write: %v", err)
		}
	*/

	scanner := bufio.NewScanner(port)
	for scanner.Scan() {
		str := scanner.Text()
		m := parse(str)
		log.Printf("%+v %q", m, str)

		switch m.Dataset {
		case "PTERM", "ITERM", "DTERM", "GYRO":
			datasets[m.Dataset] = append(datasets[m.Dataset], Vector{
				mustParseFloat(m.Payload["ROLL"]),
				mustParseFloat(m.Payload["PITCH"]),
				mustParseFloat(m.Payload["YAW"]),
			})
		}
	}
}

func JsonDataset(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")

	json.NewEncoder(w).Encode(&datasets)
}

func BinaryDataset(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/octet-stream")

	for _, d := range datasets["DTERM"] {
		binary.Write(w, binary.LittleEndian, d.Roll)
		binary.Write(w, binary.LittleEndian, d.Pitch)
		binary.Write(w, binary.LittleEndian, d.Yaw)
	}
}

func drawLine(s *svg.SVG, set, color string, offsetX, offsetY int) {
	x, y := []int{}, []int{}
	for i, d := range datasets[set] {
		y = append(y, offsetY+int(d.Roll*100))
		x = append(x, offsetX+i*25)
	}
	s.Polyline(x, y, "fill:none;stroke:"+color)
}

func SVGDataset(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "image/svg+xml")

	s := svg.New(w)
	s.Start(5000, 500)

	s.Text(0, 15, "PTERM ROLL", "fill:red")
	drawLine(s, "PTERM", "red", 0, 450)

	s.Text(0, 30, "ITERM ROLL", "fill:green")
	drawLine(s, "ITERM", "green", 0, 450)

	s.Text(0, 45, "DTERM ROLL", "fill:blue")
	drawLine(s, "DTERM", "blue", 0, 450)

	s.Text(0, 60, "GYRO ROLL", "fill:black")
	drawLine(s, "GYRO", "black", 0, 450)

	s.End()
}

func main() {
	go readSerial()

	http.Handle("/json", http.HandlerFunc(JsonDataset))
	http.Handle("/svg", http.HandlerFunc(SVGDataset))
	http.Handle("/binary", http.HandlerFunc(BinaryDataset))
	http.ListenAndServe(":8080", nil)
}
