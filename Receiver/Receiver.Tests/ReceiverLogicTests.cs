using Receiver.Data;
using Receiver.Mappings;
using System.Linq;
using System.Threading.Tasks;
using Xunit;

namespace Receiver.Tests
{
    public class ReceiverLogicTests
    {
        [Fact]
        public void HandleNext_SavesInitialTrackState()
        {
            // Arrange
            var mapper = Arrange.GetMapper();
            var driverRepository = Arrange.GetGenericRepository<Driver>();
            var sut = new ReceiverLogic(mapper, driverRepository);
            var jsonTrack = Arrange.GetJsonTrackState();

            // Act
            //sut.HandleNext(jsonTrack);
            //var vehicle = driverRepository.GetAll().FirstOrDefault();

            // Assert
            //Assert.NotNull(vehicle);
            //Assert.Equal(1, driverRepository.GetAll().Count());
            //Assert.Equal(jsonTrack.vehicles[0].driverName, vehicle.Name);


        }

        [Fact]
        public void HandleNextVehicle_ReturnsVehicleModelState()
        {
            // Arrange
            var mapper = Arrange.GetMapper();
            var driverRepository = Arrange.GetGenericRepository<Driver>();
            var sut = new ReceiverLogic(mapper, driverRepository);
            var vehicleState = Arrange.GetDrivingVehicleInSector3();
            var nextVehicleState = Arrange.GetJsonTrackState().vehicles[0];

            // Act
            //sut.HandleNextVehicleState(vehicleState, nextVehicleState);

            // Assert
            //Assert.NotNull(vehicleState);
            //Assert.Equal(vehicleState.DriverName, nextVehicleState.driverName);

        }

    }
}
