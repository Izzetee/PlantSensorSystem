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

    public class SensorDataController : ControllerBase
    {
        // GET: api/<SensorDataController>
        [HttpGet]
        public List<SensorData> Get()
        {
            var db = new SmartHomeServiceContext();
            var data = db.SensorDatas.OrderBy(b => b.Id).ToList();
            return data;
        }

        // GET api/<SensorDataController>/5
        [HttpGet("{id}")]
        public string Get(int id)
        {
            return "value";
        }

        // POST api/<SensorDataController>
        [HttpPost]
        public void Post([FromBody] string value)
        {
        }

        // PUT api/<SensorDataController>/5
        [HttpPut("{id}")]
        public void Put(int id, [FromBody] string value)
        {
        }

        // DELETE api/<SensorDataController>/5
        [HttpDelete("{id}")]
        public void Delete(int id)
        {
        }
    }
}
