import express from "express"
import morgan from "morgan"
import { createServer } from "http"
import { WebSocketServer } from "ws"
import ip from "ip"

const app = express()
const http = createServer(app)
const server = new WebSocketServer({ server: http })

// log all requests to the server
app.use(morgan("dev"))

app.get("/", (request, response) => {
    response.send("Hello there!")
})

// when something connects to this server...
server.on("connection", socket => {
    // send socket errors to the main error stream
    socket.on("error", console.error)

    // print data sent to the WebSocket server
    socket.on("message", data => {
        console.log("> " + data)

        // data is a Uint8Array, and toString() turns it into a string
        // parseFloat returns a number or NaN, which is also a number
        const datastr = data.toString()

        // if it's a relevant data packet, it will match /(LEFT|RIGHT)\s\d+\.\d+/
        const regex = /(?<dir>LEFT|RIGHT)\s(?<rpm>\d+\.\d+)/; let match
        if ((match = datastr.match(regex))) {
            let { dir, rpm } = match.groups
            const key = dir === "LEFT" ? "left_rpm" : "right_rpm"
            rpm = parseFloat(rpm)
            console.log(rpm)

            // send to the database
            fetch("https://live-timing-dash.herokuapp.com/sept_test_table", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ [key]: rpm }),
            }).then(response => {
                if (response.ok) {
                    console.log("Response was OK")
                    response.text().then(text => console.log(text))
                }
            }).catch(reason => {
                console.log(reason)
            })
        }

    })
})

// set up server exposed to other devices on the network
const port = process.env.PORT ?? 8080
http.listen(port, () => {
    console.log(`Listening at ${ip.address()}:${port}`)
})