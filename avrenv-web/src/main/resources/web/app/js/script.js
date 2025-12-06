/*
 * File:   script.js
 * Author: torsten.roemer@luniks.net
 * 
 * Leaflet is awesome!
 *
 * Created on 30.11.2025, 16:27
 */

import $ from 'jquery';

var lat = 50.70375;
var lon = 3.45252;
var zoom = 13;

var map = L.map('map');
var marker = null;
var intId = 0;

/**
 * "Initialization" done when the document is "ready".
 */
$(document).ready(function () {
    $('#center').on('click', center);
    
    map.setView([lat, lon], zoom);

    L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);
    
    window.setInterval(data, 3000);
});

/**
 * Updates data and map marker without centering the map.
 * 
 * @returns {void}
 */
function data() {
    update(false);
}

/**
 * Updates data and map marker and centers the map.
 * 
 * @returns {void}
 */
function center() {
    update(true);
}

/**
 * Gets the current data set from the service and updates the table and 
 * the map marker to the current position, and centers the map if given
 * argument is true.
 * 
 * @param {Boolean} center true if map should be centered
 * @returns {Boolean}
 */
function update(center) {
    $.ajax({
        url: '/data',
        type: 'get',
        cache: false,
        contentType: false,
        processData: false,
        success: function (data) {
            ui(data, center);
        },
        error: function (jqXHR, textStatus, errorThrown) {
            console.log('Getting data failed');
        }
    });

    return false;
}

/**
 * Updates the table and the map marker to the current position
 * with the given data, and centers the map if given argument is true.
 * 
 * @param {Json} data
 * @param {Boolean} center
 * @returns {void}
 */
function ui(data, center) {
    var online = data.online ? 'Online 🎈' : 'Offline 🙄';
    $('#online').text(online);
    
    // do nothing if baloon is not online
    if (!data.online) {
        return;
    }
    
    var crc = data.crc === 1 ? 'OK' : 'NOK';
    $('#crc').text(crc);
    
    // do nothing if radio transmission had a CRC error
    if (data.crc !== 1) {
        return;
    }
    
    var fix = data.fix !== 0 ? 'Yes' : 'No';

    $('#time').text(data.time);
    $('#dur').text(data.dur);
    $('#rssi').text(data.rssi);
    // $('#crc').text(data.crc);
    $('#voltage').text(data.voltage);
    $('#power').text(data.power);
    $('#temperature').text(data.temperature);
    $('#humidity').text(data.humidity);
    $('#pressure').text(data.pressure);
    $('#gasres').text(data.gasres);
    $('#fix').text(fix);
    $('#sat').text(data.sat);
    $('#lat').text(data.lat);
    $('#lon').text(data.lon);
    $('#alt').text(data.alt);
    $('#speed').text(data.speed);

    // only update map if GPS module has a satellite fix
    if (data.fix === 1) {
        if (center) {
            zoom = map.getZoom();
            map.setView([data.lat, data.lon], zoom);
        }

        if (marker !== null) {
            marker.remove();
        }
        marker = L.marker([data.lat, data.lon]).addTo(map);
    }

}
