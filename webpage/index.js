const url = "https://api.github.com/repos/democloid/picoTracker/releases/latest";

fetch(url)
        .then(response => response.json())
        .then(data => {
            downloadButton.textContent = 'Download Now';
        })
        .catch(error => {
            console.error('Error fetching latest release:', error);
            downloadButton.textContent = 'Failed to Fetch Download';
        });


function downloadLatest() {
    const downloadButton = document.getElementById('downloadButton');
    downloadButton.textContent = 'Downloading...';
    fetch(url)
        .then(response => response.json())
        .then(data => {
            const downloadUrl = data.assets[0].browser_download_url; // Assuming the asset you want is the first one
            window.location.href = downloadUrl;
        })
        .catch(error => {
            console.error('Error fetching latest release:', error);
            downloadButton.textContent = 'Failed to Download';
        });
}


function openSidebar() {
    document.getElementById("sidebar").style.width = "250px";
    document.getElementById("sidebar").style.right = "0";
}

function closeSidebar() {
    document.getElementById("sidebar").style.width = "0";
    document.getElementById("sidebar").style.right = "-250px";
}