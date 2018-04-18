using AutoMapper;

namespace Receiver.Mappings
{
    public class Rfactor2Profile : Profile
    {
        public Rfactor2Profile()
        {
            CreateMap<Json.TrackState, Models.Session>()
                .ForMember(session => session.Type, o => o.MapFrom(m => m.session))
                .ForMember(session => session.Duration, o => o.MapFrom(m => m.endET))
                .ForMember(session => session.CurrentTime, o => o.MapFrom(m => m.currentET));

            CreateMap<Json.VehicleState, Models.Vehicle>()
                .ForMember(destination => destination.Id, map => map.MapFrom(source => source.id))
                .ForMember(destination => destination.Name, map => map.MapFrom(source => source.vehicleName))
                .ForMember(destination => destination.DriverName, map => map.MapFrom(source => source.driverName))
                .ForMember(destination => destination.Place, map => map.MapFrom(source => source.place))
                .ForMember(destination => destination.Position, map => map.MapFrom(source => source.Pos))
                .ForMember(destination => destination.Velocity, map => map.MapFrom(source => source.metersPerSecond))
                .ForMember(destination => destination.Sector, map => map.MapFrom(source => (Models.Sector)source.sector));

            CreateMap<Json.TrackState, Models.Track>()
                .ForMember(track => track.Name, o => o.MapFrom(m => m.trackName))
                .ForMember(track => track.Distance, o => o.MapFrom(m => m.lapDist))
                .ForMember(track => track.Phase, o => o.MapFrom(m => m.gamePhase))
                .ForMember(track => track.SectorFlags, o => o.MapFrom(m => m.sectorFlags.ToString()))
                .ForMember(track => track.Session, o => o.MapFrom(m => m))
                .ForMember(track => track.Vehicles, o => o.MapFrom(source => source.vehicles));
        }


        //Unmapped members were found.Review the types and members below.
        //Add a custom mapping expression, ignore, add a custom resolver, or modify the source/destination type
        //For no matching constructor, add a no-arg ctor, add optional arguments, or map all of the constructor parameters
        //==================================================
        //TrackState -> Track (Destination member list)
        //Receiver.Json.TrackState -> Receiver.Models.Track (Destination member list)

        //Unmapped properties:
        //Name
        //Distance
        //Phase


        private void MapVehicle(Json.VehicleState source, Models.Vehicle destination)
        {
            // Set all the previous values
            destination.PreviousStatus = destination.Status;
            destination.PreviousSector = destination.Sector;
            destination.PreviousPosition = destination.Position;
            destination.PreviousVelocity = destination.Velocity;

            // Set the current values
            destination.Sector = (Models.Sector)source.sector;

            // * -1 to get rid of the negative
            // [2] is the speed we need here
            //destination.Velocity = source.localVel[2] * -1;
        }

    }
}
