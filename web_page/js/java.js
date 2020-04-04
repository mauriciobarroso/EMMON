Highcharts.chart('container1', {

    title: {
        text: null
    },

    subtitle: {
        text: 'Últimos 6 meses'
    },

    yAxis: {
        title: {
            text: 'KWh'
        }
    },

    xAxis: {
        accessibility: {
            rangeDescription: 'monday'
        },
        type: 'datetime'
    },

    legend: {
        layout: 'vertical',
        align: 'right',
        verticalAlign: 'middle'
    },

    plotOptions: {
        series: {
            label: {
                connectorAllowed: false
            },
            pointStart: Date.UTC(2020, 10, 1),
            pointInterval: Number(interval)
        }
    },
    
    series: [{
        name: 'Consumo de energía',
        data: [43934, 52503, 57177, 69658, 97031, 19931, 13713, 14175, 1111,2222,1123,32321,1,1,1,1,1,1,2,2,22,3,3,3,]}],

    responsive: {
        rules: [{
            condition: {
                maxWidth: 500
            },
            chartOptions: {
                legend: {
                    layout: 'horizontal',
                    align: 'center',
                    verticalAlign: 'bottom'
                }
            }
        }]
    }

});

Highcharts.chart('container2', {

    title: {
        text: null
    },

    subtitle: {
        text: 'Última semana'
    },

    yAxis: {
        title: {
            text: 'KWh'
        }
    },

    xAxis: {
        accessibility: {
            rangeDescription: 'monday'
        },
        type: 'datetime'
    },

    legend: {
        layout: 'vertical',
        align: 'right',
        verticalAlign: 'middle'
    },

    plotOptions: {
        series: {
            label: {
                connectorAllowed: false
            },
            pointStart: Date.UTC(2020, 10, 1),
            pointInterval: Number(interval)
        }
    },
    
    series: [{
        name: 'Consumo de energía',
        data: [Number(interval),Number(interval + 1),Number(interval),Number(interval)]}],//[43934, 52503, 57177, 69658, 97031, 19931, 13713, 14175, 1111,2222,1123,32321,1,1,1,1,1,1,2,2,22,3,3,3,]}],

    responsive: {
        rules: [{
            condition: {
                maxWidth: 500
            },
            chartOptions: {
                legend: {
                    layout: 'horizontal',
                    align: 'center',
                    verticalAlign: 'bottom'
                }
            }
        }]
    }

});