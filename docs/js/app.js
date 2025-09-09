var idToFile = [];
idToFile["nos_docs"] = "html/welcome.html";
idToFile["coding_practices"] = "html/coding_practices.html";
idToFile["var_def"] = "html/coding_practices.html#var_def";
idToFile["func_def"] = "html/coding_practices.html#func_def";
idToFile["control_flow"] = "html/coding_practices.html#control_flow";


async function fetchHtmlAsText(url) {
    return await (await fetch(url)).text();
}


async function loadContent(url) {
    const contentDiv = document.getElementById("content");
    contentDiv.innerHTML = await fetchHtmlAsText(url);
}

document.querySelector('.menu').addEventListener('click', function(event) {
  if (event.target.tagName === 'BUTTON') {
    const id = event.target.id;
    loadContent(idToFile[id]); // your function
  }
});

loadContent("html/welcome.html");