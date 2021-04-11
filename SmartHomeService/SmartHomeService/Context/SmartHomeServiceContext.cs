using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;
using SmartHomeService.Models;

namespace SmartHomeService.Context
{
    public class SmartHomeServiceContext : DbContext
    {
        public DbSet<SensorData> SensorDatas { get; set; }
        public DbSet<PlantData> PlantDatas { get; set; }

        protected override void OnConfiguring(DbContextOptionsBuilder options)
            => options.UseSqlServer("Server=<op adress>;Database=home;Trusted_Connection=True;User ID=home;Password=<password>;MultipleActiveResultSets=True;Integrated Security=false");
    }
}
