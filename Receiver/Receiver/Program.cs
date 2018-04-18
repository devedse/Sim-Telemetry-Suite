using Autofac;
using Microsoft.Extensions.Configuration;
using System;
using System.Threading.Tasks;

namespace Receiver
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Console.WriteLine("#######################################################");
            Console.WriteLine("Sim Telemetry Suite - Receiver started.");
            Console.WriteLine("Press any key to shutdown...");
            Console.WriteLine("#######################################################");

            var container = Setup.Configure();

            using (var scope = container.BeginLifetimeScope())
            {
                var hub = scope.Resolve<HubSender>();
                Task.Run(hub.Start);

                var logic = scope.Resolve<ReceiverLogic>();
                logic.Start();
            }

            Console.ReadLine();
        }
    }
}
