
window.SURVEY_URL = 'https://www.surveymonkey.com/r/JDGSDJC';
window.SHOW_SURVEY_AFTER_SECONDS = 20*60; // Show the survey after 20min of activity

window.dataLayer = window.dataLayer || [];
window.loadingSurvey = false;
window.showingSurvey = false;
window.tookSurvey = localStorage.surveyUrl === window.SURVEY_URL


// Open external links in a new tab
$(document).ready(function () {
    checkIfAcceptedCookies();
    $('a[href^="http://"], a[href^="https://"]').not('a[class*=internal]').attr('target', '_blank');
});


function gtag() {
    dataLayer.push(arguments);
}

function initialiseGoogleAnalytics() {
    console.log('Loading google analytics');
    gtag('js', new Date());
    gtag('config', gTrackingId, {'anonymize_ip': true});
}

function checkIfAcceptedCookies() {
    if (!localStorage.acceptedCookies) {
        $('.privacy-banner').show();
        $('.privacy-banner-accept').click(function() {
            $('.privacy-banner').hide()
            localStorage.acceptedCookies = 'true';
            onAcceptedCookies();
        });
        
    } else {
        onAcceptedCookies();
    }
}

function onAcceptedCookies() {
    initialiseGoogleAnalytics();
    $('#survey-link').on('click', function() {
        window.tookSurvey = false;
        window.loadingSurvey = false;
        window.showingSurvey = false;
        showSurvey();
    });
    if(!window.tookSurvey) {
        localStorage.lastActivityTimestamp = Date.now() / 1000;
        if(!localStorage.activeSeconds) {
            localStorage.activeSeconds = 0;
        }
        $(window).scroll(updateActivity);
        $(document).on('mousemove', updateActivity);
    }
}

function updateActivity() {
    let now = Date.now() / 1000;
    let elapsed = now - parseFloat(localStorage.lastActivityTimestamp);
    
    if(!elapsed || elapsed < 1) { 
        return
    }
    localStorage.lastActivityTimestamp = now;

    if(elapsed > 5*60) { // If more than 5min elapsed, then assume the user walked away so ignore this activity
        return;
    }

    let totalSeconds = parseFloat(localStorage.activeSeconds) + elapsed;
    localStorage.activeSeconds = totalSeconds;
    if(totalSeconds >= window.SHOW_SURVEY_AFTER_SECONDS) {
        showSurvey();
    }
}


function showSurvey() {
    if(localStorage.acceptedCookies && !window.tookSurvey && !window.loadingSurvey) {
        window.loadingSurvey = true;

        $('#iframe-survey').on('load', function() {
            if(window.showingSurvey) {
                closeSurvey();
            } else {
                window.showingSurvey = true;
                $('#dlg-survey').css('display', 'block');
            }
        });
        $('#iframe-survey').attr('src', window.SURVEY_URL);
        $("#dlg-survey-close").on("click", closeSurvey);
    }
}

function closeSurvey() {
    console.info('Took survey');
    window.tookSurvey = true;
    localStorage.surveyUrl = window.SURVEY_URL;
    $('#dlg-survey').css('display', 'none');
    $('#iframe-survey').off('load');
    $('#iframe-survey').attr('src', '');
}
