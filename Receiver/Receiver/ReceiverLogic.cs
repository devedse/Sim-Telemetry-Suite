using AutoMapper;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Receiver.Data;
using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Reactive;
using System.Reactive.Linq;
using System.Reactive.Threading.Tasks;
using System.Text;

namespace Receiver
{
    public class ReceiverLogic
    {
        private Models.Track _track;
        private readonly IMapper _mapper;
        private readonly IGenericRepository<Driver> _driverRepository;
        private readonly HubSender _hub;

        public ReceiverLogic(IMapper mapper, IGenericRepository<Driver> driverRepository, HubSender hub)
        {
            _mapper = mapper;
            _driverRepository = driverRepository;
            _hub = hub;
        }

        public void Start()
        {
            // Set up the udp endpoint
            var endpoint = new IPEndPoint(IPAddress.Any, 6666);

            // Create the Observer
            var observer = Observer.Create<string>(HandleJson, HandleException, HandleCompleted);

            // Set up the udp stream
            UdpStream(endpoint).Subscribe(observer);
        }

        private void HandleJson(string json)
        {
            // If you are debugging and want to see every message in the console... :)
            //Console.Clear();
            //Console.WriteLine("Remember; close this window by pressing any key ...\n");
            //var formattedJson = JValue.Parse(json).ToString(Formatting.Indented);
            //Console.WriteLine(formattedJson);

            var trackState = JsonConvert.DeserializeObject<Json.TrackState>(json);

            // First handle the track state
            if (_track == null)
            {
                _track = _mapper.Map<Models.Track>(trackState);
            }

            // Then handle all the vehicle states
            foreach (var vehicle in trackState.vehicles)
            {
                HandleNextVehicleState(_track, vehicle);

                // ######################
                // Experimental DB stuff
                // ######################

                //var foundDriver = await _driverRepository.GetByName(jsonVehicle.driverName);
                //if (foundDriver != null)
                //{
                //    // Do not add the driver, it already exists
                //    continue;
                //}

                //await _driverRepository.Create(new Driver
                //{
                //    Name = jsonVehicle.driverName
                //});

                // ######################
            }

            _hub.SendStatus(_track);
        }

        private void HandleException(Exception exception)
        {
            Console.WriteLine($"Error: {exception.Message}");
        }

        private void HandleCompleted()
        {
            Console.WriteLine("Completed.");
        }

        public void HandleNextVehicleState(Models.Track track, Json.VehicleState jsonVehicle)
        {
            var vehicle = track.Vehicles.FirstOrDefault(v => v.Id == jsonVehicle.id);
            if (vehicle == null)
            {
                Console.WriteLine($"{jsonVehicle.driverName} joined the server");
                vehicle = _mapper.Map<Models.Vehicle>(jsonVehicle);
                track.Vehicles.Add(vehicle);
            }
            else
            {
                // Update vehicle
                vehicle.PreviousPosition = vehicle.Position;
                vehicle.PreviousSector = vehicle.Sector;
                vehicle.PreviousStatus = vehicle.Status;
                vehicle.PreviousVelocity = vehicle.Velocity;
                _mapper.Map(jsonVehicle, vehicle);
            }

            // Get the current lap
            var lapNumber = jsonVehicle.totalLaps + 1;
            var currentLap =
                vehicle.Laps.FirstOrDefault(l => l.Number == lapNumber) ??
                new Models.Lap { Number = lapNumber };

            // Update current lap time and path
            currentLap.Time = jsonVehicle.last;
            if (!currentLap.Path.ContainsKey((int)jsonVehicle.lapDist) && currentLap.Time != null)
            {
                // Add the vehicle position to it's path
                currentLap.Path.Add((int)jsonVehicle.lapDist, jsonVehicle.Pos);
            }

            // Calculate the top speed
            vehicle.TopVelocity = vehicle.Velocity > vehicle.TopVelocity ? vehicle.Velocity : vehicle.TopVelocity;

            // Watch the sectors and determine where the vehicle is
            if (vehicle.PreviousSector == Models.Sector.Sector3 &&
                vehicle.Sector == Models.Sector.Sector1 &&
                vehicle.NewLap == false)
            {
                if (jsonVehicle.inPits == 1 && vehicle.Status != Models.Status.Pit)
                {
                    Console.WriteLine($"{vehicle.DriverName}: entered the pits");
                    vehicle.Status = Models.Status.Pit;
                }
                else
                {
                    vehicle.NewLap = true;
                    Console.WriteLine($"{vehicle.DriverName}: crossed start/finish");

                    // Finish the current lap
                    Console.WriteLine($"{vehicle.DriverName}: Lap {currentLap.Number}: {currentLap.TimeString}");

                    // Create a new lap
                    vehicle.Laps.Add(new Models.Lap { Number = lapNumber });
                }
            }
            else if (vehicle.PreviousSector == Models.Sector.Sector1 &&
                vehicle.Sector == Models.Sector.Sector2 &&
                vehicle.NewLap == true)
            {
                vehicle.NewLap = false;
                Console.WriteLine($"{vehicle.DriverName}: sector 1: {currentLap.Sector1}");
            }
            else if (vehicle.PreviousSector == Models.Sector.Sector2 &&
                vehicle.Sector == Models.Sector.Sector3 &&
                vehicle.NewLap == false)
            {
                Console.WriteLine($"{vehicle.DriverName}: sector 2: {currentLap.Sector2}");
            }
        }

        /// <summary>
        /// Set up an observable udp stream
        /// </summary>
        /// <typeparam name="T">Type of message</typeparam>
        /// <param name="endpoint">The endpoint to listen to</param>
        /// <param name="processor">Function that handles the resulting buffer</param>
        /// <returns></returns>
        private IObservable<string> UdpStream(IPEndPoint endpoint)
        {
            return Observable.Using(
                () => new UdpClient(endpoint),
                (udpClient) =>
                    Observable.Defer(() => udpClient.ReceiveAsync().ToObservable())
                    .Repeat()
                    .Select((result) =>
                    {
                        return Encoding.UTF8.GetString(result.Buffer).Trim('\0');
                    })
            );
        }
    }
}
