package main

import (
	"bufio"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"strconv"
	"strings"
	"github.com/rs/cors"
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
			if pos == size-1 || s[pos] == ',' {
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

const datasetLimit = 100

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

	if _, err := port.Write([]byte{'T'}); err != nil {
		log.Fatalf("port.Write: %v", err)
	}

	runup := 10
	scanner := bufio.NewScanner(port)
	for scanner.Scan() {
		if runup > 0 {
			runup--
			continue
		}

		str := scanner.Text()
		m := parse(str)
		log.Printf("%+v %q", m, str)

		switch m.Dataset {
		case "PTERM", "ITERM", "DTERM", "GYRO", "OUTPUT", "ANGLE_PTERM", "ANGLE_DTERM":
			if len(datasets[m.Dataset]) > datasetLimit {
				datasets[m.Dataset] = datasets[m.Dataset][:1]
			}

			datasets[m.Dataset] = append(datasets[m.Dataset], Vector{
				mustParseFloat(m.Payload["ROLL"]),
				mustParseFloat(m.Payload["PITCH"]),
				mustParseFloat(m.Payload["YAW"]),
			})
		case "ACCEL", "ANGLE", "STICK":
			if len(datasets[m.Dataset]) > datasetLimit {
				datasets[m.Dataset] = datasets[m.Dataset][:1]
			}

			datasets[m.Dataset] = append(datasets[m.Dataset], Vector{
				mustParseFloat(m.Payload["Y"]),
				mustParseFloat(m.Payload["X"]),
				mustParseFloat(m.Payload["Z"]),
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
		y = append(y, offsetY+int(d.Pitch*250))
		x = append(x, offsetX+i*10)
	}
	s.Polyline(x, y, "fill:none;stroke:"+color)
}

func SVGDataset(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "image/svg+xml")

	s := svg.New(w)
	s.Start(5000, 1000)

	/*
	s.Text(0, 15, fmt.Sprintf("GYRO ROLL %f", datasets["GYRO"][len(datasets["GYRO"])-1].Pitch), "fill:red")
	drawLine(s, "GYRO", "red", 0, 500)

	s.Text(0, 30, fmt.Sprintf("ACCEL ROLL %f", datasets["ACCEL"][len(datasets["ACCEL"])-1].Pitch), "fill:green")
	drawLine(s, "ACCEL", "green", 0, 500)
	*/

	s.Text(0, 15, fmt.Sprintf("ANGLE ROLL %f", datasets["ANGLE"][len(datasets["ANGLE"])-1].Pitch), "fill:black")
	drawLine(s, "ANGLE", "black", 0, 500)

	s.Text(0, 30, fmt.Sprintf("STICK ROLL %f", datasets["STICK"][len(datasets["STICK"])-1].Pitch), "fill:blue")
	drawLine(s, "STICK", "blue", 0, 500)

	s.Text(0, 45, fmt.Sprintf("PTERM ROLL %f", datasets["ANGLE_PTERM"][len(datasets["ANGLE_PTERM"])-1].Pitch), "fill:red")
	drawLine(s, "ANGLE_PTERM", "red", 0, 500)

	s.Text(0, 60, fmt.Sprintf("DTERM ROLL %f", datasets["ANGLE_DTERM"][len(datasets["ANGLE_DTERM"])-1].Pitch), "fill:green")
	drawLine(s, "ANGLE_DTERM", "green", 0, 500)

	s.Text(0, 75, fmt.Sprintf("OUTPUT ROLL %f", datasets["OUTPUT"][len(datasets["OUTPUT"])-1].Pitch), "fill:orange")
	drawLine(s, "OUTPUT", "orange", 0, 500)

	s.End()
}

func main() {
	go readSerial()

	mux := http.NewServeMux()
	mux.Handle("/json", http.HandlerFunc(JsonDataset))
	mux.Handle("/svg", http.HandlerFunc(SVGDataset))
	mux.Handle("/binary", http.HandlerFunc(BinaryDataset))

	handler := cors.Default().Handler(mux)
	http.ListenAndServe(":8080", handler)
}
