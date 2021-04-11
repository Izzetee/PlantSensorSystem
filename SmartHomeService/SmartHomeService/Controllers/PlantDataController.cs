using Microsoft.AspNetCore.Mvc;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using SmartHomeService.Context;
using SmartHomeService.Models;

// For more information on enabling Web API for empty projects, visit https://go.microsoft.com/fwlink/?LinkID=397860

namespace SmartHomeService.Controllers
{
    [ApiController]
    [Route("api/[controller]")]

    public class PlantDataController : ControllerBase
    {
        // GET: api/<PlantDataController>
        [HttpGet]
        public List<PlantData> Get()
        {
            var db = new SmartHomeServiceContext();
            var data = db.PlantDatas.OrderBy(b => b.Id).ToList();
            return data;
        }

        // GET api/<PlantDataController>/5
        [HttpGet("{id}")]
        public PlantData Get(int id)
        {
            var db = new SmartHomeServiceContext();
            PlantData data = db.PlantDatas.FirstOrDefault(e => e.Id == id);
            data.SensorDataHistory = db.SensorDatas.Where(e => e.SensorId == data.SensorId).ToList();
            return data;
        }

        // POST api/<PlantDataController>
        [HttpPost]
        public void Post([FromBody] string value)
        {
        }

        // PUT api/<PlantDataController>/5
        [HttpPut("{id}")]
        public void Put(int id, [FromBody] string value)
        {
        }

        // DELETE api/<PlantDataController>/5
        [HttpDelete("{id}")]
        public void Delete(int id)
        {
        }
    }
}
