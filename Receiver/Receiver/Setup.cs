using Autofac;
using Autofac.Configuration;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Receiver.Data;
using Receiver.Mappings;
using System;
using System.Collections.Generic;
using System.IO;

namespace Receiver
{
    public static class Setup
    {
        public static IContainer Configure()
        {
            var config = ConfigureApplicationSettings();
            return ConfigureDependencyInjection(config);
        }

        public static IConfiguration ConfigureApplicationSettings()
        {
            var builder = new ConfigurationBuilder()
                .SetBasePath(Directory.GetCurrentDirectory())
                .AddJsonFile("appsettings.json", optional: false, reloadOnChange: true);

            return builder.Build();
        }

        public static DbContextOptions<T> ConfigureDbContextOptions<T>(IConfiguration config)
            where T : DbContext
        {
            return new DbContextOptionsBuilder<T>()
                .UseSqlite(config.GetConnectionString("DefaultConnection"))
                .Options;
        }

        /// <summary>
        /// Autofac: Dependency Injection
        /// </summary>
        /// <param name="config"></param>
        /// <param name="mapper"></param>
        /// <returns></returns>
        private static IContainer ConfigureDependencyInjection(IConfiguration config)
        {
            var autofacBuilder = new ContainerBuilder();

            // DB Context
            var dbContextOptions = ConfigureDbContextOptions<TelemetryContext>(config);
            autofacBuilder.Register(b => new TelemetryContext(dbContextOptions)).SingleInstance();

            // Custom types
            autofacBuilder.Register(c => config).SingleInstance();
            autofacBuilder.RegisterGeneric(typeof(GenericRepository<>)).As(typeof(IGenericRepository<>));
            autofacBuilder.RegisterType<IMapper>();
            autofacBuilder.RegisterType<HubSender>().AsSelf().SingleInstance();
            autofacBuilder.RegisterType<ReceiverLogic>();

            // Register all mappers
            var assemblies = AppDomain.CurrentDomain.GetAssemblies();
            autofacBuilder.RegisterAssemblyTypes(assemblies)
                .Where(t => typeof(IMapper).IsAssignableFrom(t) && !t.IsAbstract && t.IsPublic)
                .As<IMapper>();

            // Register mapper
            //autofacBuilder.Register(c => c.Resolve<MapperConfiguration>().CreateMapper(c.Resolve)).As<IMapper>().InstancePerLifetimeScope();

            return autofacBuilder.Build();
        }
    }
}
