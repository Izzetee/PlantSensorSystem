using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations.Schema;

namespace SmartHomeService.Models
{
    public class SensorData
    {
        [DatabaseGenerated(DatabaseGeneratedOption.Identity)]
        public int Id { get; set; }

        public int SensorId { get; set; }

        public int SoilMoistPercentage { get; set; }

        public decimal AirTemp { get; set; }

        public decimal AirMoist { get; set; }

        public decimal AirPressure { get; set; }

        public DateTime Timestamp { get; set; }
    }
}
