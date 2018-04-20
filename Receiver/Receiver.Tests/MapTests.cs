using Xunit;

namespace Receiver.Tests
{
    public class MapTests
    {
        [Fact]
        public void MapTrack_ReturnsFullyMappedTrackModel()
        {
            // Arrange
            var sut = Arrange.GetMapper();
            var jsonTrack = Arrange.GetJsonTrackState();

            // Act
            var track = sut.MapTrack(jsonTrack);

            // Assert
            Assert.NotNull(track);
            Assert.NotNull(track.Vehicles);
            Assert.NotEmpty(track.Vehicles);
            Assert.Equal(2, track.Vehicles.Count);
            Assert.Equal(jsonTrack.trackName, track.Name);
            Assert.Equal(jsonTrack.lapDist, track.Distance);
        }

        // Map Test idee
        // !!!! Track State, we gaan over States praten, en niet zomaar over losse flying objects. maakt geen sense.
        // .. map the given track json to a track state object
    }
}
