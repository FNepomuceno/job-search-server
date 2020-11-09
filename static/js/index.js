let request_button = document.getElementById("job_request");
request_button.addEventListener("click", (event) => {
    // convert this into an asynchronous function TODO
    console.log("Getting applications...");
    let httpRequest = new XMLHttpRequest();
    if (!httpRequest) return;

    httpRequest.onreadystatechange = function () {
        if (httpRequest.readyState === XMLHttpRequest.DONE
                && httpRequest.status == 200) {
            let json_response = JSON.parse(httpRequest.responseText);
            let rows = json_response.result;
            console.log("Applications received.");

            // Clear out table
            let row_parent =
                document.getElementById("jobs_columns").parentElement;
            while (row_parent.firstElementChild !=
                    row_parent.lastElementChild) {
                row_parent.removeChild(row_parent.lastElementChild);
            }

            // Add results
            if (rows.length) {
                for (let row of rows) {
                    let new_row = document.createElement("tr");
                    for (let value of Object.values(row)) {
                        let new_cell = document.createElement("td");
                        new_cell.innerHTML = value;
                        new_row.appendChild(new_cell);
                    }
                    row_parent.appendChild(new_row);
                }
            } else {
                let new_row = document.createElement("tr");
                let new_cell = document.createElement("td");
                new_cell.innerHTML = "No applications found";
                new_cell.colSpan = 12;
                new_row.appendChild(new_cell);
                row_parent.appendChild(new_row);
            }
        }
    };

    httpRequest.open('GET', 'jobs');
    httpRequest.send();
});
