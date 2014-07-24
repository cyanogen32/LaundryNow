
 var http = require('http'); 
 var mongoose = require('mongoose'); 
 var url = require('url'); 
 


/* Database Set-Up */

 //mongoose.connect('mongodb://localhost/laundry-db');
  mongoose.connect('mongodb://nodejitsu:9af84d2fde893a3b113393e4482658dc@paulo.mongohq.com:10027/nodejitsudb1103301669');

 db = mongoose.connection;

 var monitorSchema = new mongoose.Schema({
 	syncCode : String, 
 	zeroPulse : String, 
 	status : Number, 
 	pulseArr : Array
 }); 

var monitor = db.model('monitor', monitorSchema);


/* Arduino
=================================*/ 
exports.recieve_pulse = function(req, res){ 
	var query = url.parse(req.url, true).query; 
	var pulse = query.pulse;  
	var sync = query.model; 
	console.log(pulse); 
	res.send('pulse recieved'); 
	monitor.findOne({syncCode : sync}, function(err, mtr){
		if(err) console.log(err); 
		mtr.pulseArr.push(pulse);
		mtr.save();
		comparePulse(mtr, function(msg, mtr){ 
			var i = mtr.pulseArr.length -1; 
			if(i>2) {console.log(mtr.pulseArr[i] + ' ' + mtr.pulseArr[i-1] + ' '  + mtr.pulseArr[i-2]); }
			console.log(msg);
		}); 
	}); 
 
}; 

comparePulse = function(mtr, callback){ 
	var i = mtr.pulseArr.length -1; 
	if((i > 10) && (mtr.zeroPulse === mtr.pulseArr[i]) && (mtr.zeroPulse === mtr.pulseArr[i-1]) && (mtr.zeroPulse === mtr.pulseArr[i-2])){ 
		changeStatus(mtr, 0, callback, 'Load Complete'); 
	}
	else{ 
		changeStatus(mtr, 1, callback, 'In Progress'); 
	}
}; 

changeStatus = function(monitor, sts, callback, msg){ 
	monitor.status = sts;  
	monitor.save();
	console.log(monitor.status);
	callback(msg, monitor);
	
}


/* User
==================================*/

exports.index = function(req, res){
	console.log('/index requested');
	if(req.session._id != undefined) {
		res.render('index' , {monitor : req.session._id});
	}
	else{
		res.render('index' , {monitor : undefined});
	}
	
};

exports.initialize = function(req, res){
	monitor.findOne({syncCode : req.param('sync')}, function(err, mtr){
		if (err) console.log(err); 
		if(mtr == undefined){ 
			var newMonitor = new monitor({
				syncCode : req.param('sync'),
				status : '-1',
			 	zeroPulse : '0',  
			 	pulseArr : []
			});

			newMonitor.save();
		}else{
			mtr.status = '-1';
			mtr.pulseArr = [];  
			mtr.save();
		}
		
		res.send('Arduino Synced'); 
		req.session._id = req.param('sync'); 
		console.log(req.session._id); 
	});


}; 


exports.set_arduino = function(req, res){
	if(req.session._id != undefined){ 
		monitor.findOne({syncCode : req.session._id}, function(err, mtr){
			mtr.zeroPulse = '49'; 
			mtr.save();  
		}); 
		res.redirect('/');
	}
};

exports.check_status = function(req, res){ 
	monitor.findOne({syncCode : req.session._id}, function(err, mtr){
			if(mtr != undefined) res.send({status : mtr.status});
			else res.send({status : -1})
			}); 
};




