using AutoMapper;
using Microsoft.EntityFrameworkCore;
using Receiver.Data;
using Receiver.Mappings;
using System;
using System.Collections.Generic;
using System.Text;

namespace Receiver.Tests
{
    public class Arrange
    {
        public static IGenericRepository<TEntity> GetGenericRepository<TEntity>() where TEntity : class, IEntity
        {
            var entityName = typeof(TEntity).Name;
            var options = new DbContextOptionsBuilder<TelemetryContext>()
                .UseSqlite($"Data Source={entityName}.unittest.db")
                .Options;

            TelemetryContext context = new TelemetryContext(options);
            context.Database.EnsureDeleted();
            context.Database.Migrate();

            return new GenericRepository<TEntity>(context);
        }

        public static IMapper GetMapper()
        {
            var mapperConfiguration = new MapperConfiguration(config =>
            {
                config.AddProfile<Rfactor2Profile>();
            });

            return mapperConfiguration.CreateMapper();
        }

        public static Json.TrackState GetJsonTrackState()
        {
            return new Json.TrackState
            {
                application = "rfactor2",
                type = "scoring",
                trackName = "track",
                session = 0,
                numVehicles = 2,
                maxLaps = 100,
                currentET = 500,
                endET = 108000,
                lapDist = 5000,
                inRealTime = 0,
                darkCloud = 0,
                raining = 0,
                ambientTemp = 30,
                trackTemp = 30,
                wind = new[] { 0f, 0f, 0f },
                minPathWetness = 0,
                maxPathWetness = 0,
                vehicles = new[]
                {
                    new Json.VehicleState {
                        id = 0,
                        driverName = "test",
                        vehicleName = "auto"
                    },
                    new Json.VehicleState
                    {
                        id = 100,
                        driverName = "test2",
                        vehicleName = "auto2"
                    }
                }
            };
        }

        public static Models.Vehicle GetDrivingVehicleInSector3()
        {
            return new Models.Vehicle
            {
                Id = 0,
                DriverName = "test",
                Name = "auto",
                Position = new[] { 4000f, 4000f, 4000f },
                Sector = Models.Sector.Sector3,
                Status = Models.Status.Driving,
                Velocity = 60f
            };
        }

        public static Json.VehicleState GetVehicleStateInSector1()
        {
            return new Json.VehicleState
            {
                id = 0,
                driverName = "test",
                inPits = 0,
                Pos = new[] { 4000f, 4010f, 10f },
                sector = 1,
                totalLaps = 1,
                vehicleName = "auto",
                metersPerSecond = 31f
            };
        }
    }
}
