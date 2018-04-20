using System;
using System.Collections.Generic;
using System.Text;

namespace Receiver.Mappings
{
    public class RFactor2Mapper : IMapper
    {
        public void MapTrack(Models.Track track, Json.TrackState json)
        {
            if (track == null) { throw new ArgumentNullException("track", "Track should not be null in a mapper method."); }

            track.Name = json.trackName;
            track.Distance = json.lapDist;
            track.Phase = json.gamePhase;
            track.SectorFlags = json.sectorFlags != null ? json.sectorFlags.ToString() : string.Empty;
            track.Session = MapSession(json);
        }

        public Models.Session MapSession(Json.TrackState json)
        {
            return new Models.Session
            {
                Type = json.session,
                Duration = json.endET,
                CurrentTime = json.currentET
            };
        }

        public void MapVehicle(Models.Vehicle vehicle, Json.VehicleState json)
        {
            if (vehicle == null) { throw new ArgumentNullException("vehicle", "Vehicle should not be null in a mapper method."); }

            vehicle.Id = json.id;
            vehicle.Name = json.vehicleName;
            vehicle.DriverName = json.driverName;
            vehicle.Place = json.place;
            vehicle.Position = json.Pos;
            vehicle.Velocity = json.metersPerSecond;
            vehicle.Sector = (Models.Sector)json.sector;
        }
    }
}
