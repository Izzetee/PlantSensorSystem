using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Net;
using System.Threading.Tasks;
using SmartHomeService.Context;
using SmartHomeService.Models;

namespace SmartHomeService.Services
{
    public class Webhook : IDisposable
    {
        private readonly WebClient dWebClient;
        private static NameValueCollection discordValues;
        public string WebHook { get; set; }
        public string UserName { get; set; }
        public string ProfilePicture { get; set; }

        public const int GiessModifier = 18;

        public Webhook()
        {
            dWebClient = new WebClient();
        }

        public void SendMessage(SensorData msgSend)
        {
            SmartHomeServiceContext context = new SmartHomeServiceContext();
            var plantdata = context.PlantDatas.FirstOrDefault(e => e.SensorId == msgSend.SensorId);

            discordValues = new NameValueCollection();

            if (plantdata.TargetMoisture > msgSend.SoilMoistPercentage)
            {
                string message = $"Achtung! Bitte Gießen {plantdata.PlantName} im {plantdata.Room}! Bodenfeuchtigkeit ist {msgSend.SoilMoistPercentage}% und damit UNTER der eingestellten Feuchtigkeit von: {plantdata.TargetMoisture}%";

                discordValues.Add("username", UserName);
                discordValues.Add("avatar_url", ProfilePicture);

                discordValues.Add("content", message);
                dWebClient.UploadValues(WebHook, discordValues);
                
            }

            context = new SmartHomeServiceContext();
            var lastSensorData = context.SensorDatas
                .Where(x => x.SensorId == msgSend.SensorId && x.Timestamp != msgSend.Timestamp)
                .OrderBy(y => y.Timestamp).LastOrDefault();
            var istGegossen = lastSensorData.SoilMoistPercentage + GiessModifier;
            if (istGegossen < msgSend.SoilMoistPercentage)
            {
                discordValues = new NameValueCollection();

                string message = $"Danke für das Gießen von: {plantdata.PlantName} im {plantdata.Room}! Bodenfeuchtigkeit ist nun {msgSend.SoilMoistPercentage}% und damit ÜBER der eingestellten Feuchtigkeit von: {plantdata.TargetMoisture}%";

                discordValues.Add("username", UserName);
                discordValues.Add("avatar_url", ProfilePicture);

                discordValues.Add("content", message);
                dWebClient.UploadValues(WebHook, discordValues);
            }
        }

        public void Dispose()
        {
            dWebClient.Dispose();
        }
    }
}
