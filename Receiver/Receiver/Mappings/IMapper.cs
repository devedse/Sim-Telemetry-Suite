using System;
using System.Collections.Generic;
using System.Text;

namespace Receiver.Mappings
{
    public interface IMapper
    {
        void MapTrack(Models.Track track, Json.TrackState json);

        Models.Session MapSession(Json.TrackState json);

        void MapVehicle(Models.Vehicle vehicle, Json.VehicleState json);
    }
}
