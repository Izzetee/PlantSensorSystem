using System;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using EasyNetQ;
using EasyNetQ.Topology;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using SmartHomeService.Context;
using SmartHomeService.Models;

namespace SmartHomeService.Services
{
    public class SmartHome : IHostedService
    {
        private readonly ILogger<SmartHome> _logger;

        public SmartHome(ILogger<SmartHome> logger)
        {
            _logger = logger;
        }


        static void HandleTextMessage(int textMessage)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("Got message: {0}", textMessage);
            Console.ResetColor();
        }

        public Task StartAsync(CancellationToken cancellationToken)
        {
            _logger.LogInformation($"Loaded");
            try
            {
                var advancedBus = RabbitHutch.CreateBus("host=<ipadress>:5672;virtualHost=/;username=home;password=<password>").Advanced;

                //_bus.PubSub.Subscribe<int>("Sensor1", HandleTextMessage);
                var queue = advancedBus.QueueDeclare("Sensor1");
                var exchange = advancedBus.ExchangeDeclare("amq.topic", ExchangeType.Topic);
                var binding = advancedBus.Bind(exchange, queue, "#");

                advancedBus.Consume(queue, (body, properties, info) => Task.Factory.StartNew(() =>
                {
                    var message = Encoding.UTF8.GetString(body).Split(",");
                    var db = new SmartHomeServiceContext();
                    var data = new SensorData();
                    data.SoilMoistPercentage = Convert.ToInt32(message[0]);
                    data.Timestamp = DateTime.Now;
                    data.SensorId = Convert.ToInt32(message[1]);
                    data.AirMoist = Convert.ToDecimal(message[4]);
                    data.AirTemp = Convert.ToDecimal(message[2]);
                    data.AirPressure = Convert.ToDecimal(message[3]);
                    db.Add(data);
                    db.SaveChanges();
                    Console.WriteLine("Got message: '{0}'", message);

                    using (Webhook dcWeb = new Webhook())
                    {
                        dcWeb.ProfilePicture = "https://i.ibb.co/GRGPWwV/DSC2453.jpg";
                        dcWeb.UserName = "Sammy";
                        dcWeb.WebHook = "<apiwebhook>";
                        dcWeb.SendMessage(data);
                        dcWeb.Dispose();
                    }
                }));

            }
            catch (Exception ex)
            {

                Console.WriteLine(ex.Message);
            }
            return Task.CompletedTask;
        }

        public Task StopAsync(CancellationToken cancellationToken)
        {
            return Task.CompletedTask;
        }
    }
}
