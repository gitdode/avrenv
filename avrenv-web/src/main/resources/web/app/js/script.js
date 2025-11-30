import $ from 'jquery';

var lat = 50.71020;
var lon = 3.45672;
var zoom = 13;

var map = L.map('map');
var marker = null;

function update(center) {
    $.ajax({
        url: '/data',
        type: 'get',
        // dataType : 'text',
        cache: false,
        contentType: false,
        processData: false,
        success: function (data) {
            $('#time').text(data.time);
            $('#dur').text(data.dur);
            $('#rssi').text(data.rssi);
            $('#crc').text(data.crc);
            $('#voltage').text(data.voltage);
            $('#power').text(data.power);
            $('#temperature').text(data.temperature);
            $('#humidity').text(data.humidity);
            $('#pressure').text(data.pressure);
            $('#gasres').text(data.gasres);
            $('#fix').text(data.fix);
            $('#sat').text(data.sat);
            $('#lat').text(data.lat);
            $('#lon').text(data.lon);
            $('#alt').text(data.alt);
            $('#speed').text(data.speed);

            if (data.lat !== 0 && data.lon !== 0) {
                if (center) {
                    map.setView([data.lat, data.lon], 13);
                }

                if (marker !== null) {
                    marker.remove();
                }
                marker = L.marker([data.lat, data.lon]).addTo(map);
            }
        },
        error: function (xhr, ajaxOptions, thrownError) {
            console.log(thrownError + ': ' + xhr.responseText);
        }
    });

    return false;
}

function data() {
    update(false);
}

function center() {
    update(true);
}

function auto() {
    if ($('#auto').is(':checked')) {
        window.setInterval(data, 3000);
    } else {
        window.clearInterval();
    }
}

/**
 * Functions to get data from the rest endpoint.
 */
$(document).ready(function () {
    $('#update').on('click', data);
    $('#auto').on('click', auto);
    $('#center').on('click', center);

    $('#auto').prop('checked', false);

    map.setView([lat, lon], zoom);

    L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);
});