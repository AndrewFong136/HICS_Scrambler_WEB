const loading = document.getElementById('loading');
function displayData(rows) {
    const teamsList = document.getElementById('teamsList');
    teamsList.innerHTML = ""; 
    rows.forEach(row => {
        const tr = document.createElement('tr');

        const columns = [
            row.Members,
            row.Situation_Analyst,
            row.Solutions,
            row.Kabyas_Lapdog,
            row.Financials
        ];
        
        columns.forEach(cell => {
            const td = document.createElement('td');
            td.textContent = cell || "-";
            tr.appendChild(td);
        });
        teamsList.appendChild(tr);
    });
}

function fetchData() {
    fetch('http://localhost:5000/api/data')
        .then(response => response.json())
        .then(data => displayData(data))
}

function refreshData() {
    fetch('http://localhost:5000/api/refresh')
        .then(fetchData())
}

document.addEventListener('DOMContentLoaded', () => {
    refreshData();
});

setInterval(refreshData, 300000)