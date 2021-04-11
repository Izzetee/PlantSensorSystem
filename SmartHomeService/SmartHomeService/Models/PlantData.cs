using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations.Schema;

namespace SmartHomeService.Models
{
    public class PlantData
    {
        [DatabaseGenerated(DatabaseGeneratedOption.Identity)]
        public int Id { get; set; }

        public List<SensorData> SensorDataHistory { get; set; }

        public string Room { get; set; }

        public string PlantName { get; set; }

        public int TargetMoisture { get; set; }

        public int SensorId { get; set; }

        public DateTime LastEdited { get; set; }
    }
}
