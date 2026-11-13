(function () {
  "use strict";

  var state = {
    totalAmount: 50000,
    roundingUnit: 500,
    groups: [
      { id: "group-1", name: "多め", ratio: 1.5, count: 2 },
      { id: "group-2", name: "普通", ratio: 1.0, count: 6 },
      { id: "group-3", name: "少なめ", ratio: 0.5, count: 2 }
    ],
    calculationResult: null,
    collectionMembers: []
  };

  var nextGroupId = 4;

  function isPositiveInteger(value) {
    return Number.isInteger(value) && value >= 1;
  }

  function validateSession(session) {
    var errors = [];

    if (!isPositiveInteger(session.totalAmount)) {
      errors.push("会計総額は1以上の整数で入力してください。");
    }
    if (!isPositiveInteger(session.roundingUnit)) {
      errors.push("丸め単位は1以上の整数で入力してください。");
    }
    if (!Array.isArray(session.groups) || session.groups.length < 1) {
      errors.push("負担区分を1件以上入力してください。");
    }

    session.groups.forEach(function (group) {
      if (!group.name || group.name.trim() === "") {
        errors.push("区分名を入力してください。");
      }
      if (!Number.isFinite(group.ratio) || group.ratio <= 0) {
        errors.push("比率は0より大きい数値で入力してください。");
      }
      if (!isPositiveInteger(group.count)) {
        errors.push("人数は1以上の整数で入力してください。");
      }
    });

    return errors;
  }

  function calculateWarikan(session) {
    var errors = validateSession(session);
    if (errors.length > 0) {
      return { errors: errors, result: null };
    }

    var weightedCount = session.groups.reduce(function (sum, group) {
      return sum + group.ratio * group.count;
    }, 0);
    var baseAmount = session.totalAmount / weightedCount;
    var rows = session.groups.map(function (group) {
      var rawAmount = baseAmount * group.ratio;
      var amountPerPerson = Math.round(rawAmount / session.roundingUnit) * session.roundingUnit;
      return {
        groupId: group.id,
        groupName: group.name,
        ratio: group.ratio,
        count: group.count,
        amountPerPerson: amountPerPerson,
        subtotal: amountPerPerson * group.count
      };
    });

    var result = buildCalculationResult(session.totalAmount, rows);
    if (result.difference !== 0) {
      result = adjustDifference(session, result);
    }

    return { errors: [], result: result };
  }

  function buildCalculationResult(totalAmount, rows) {
    var totalCalculated = rows.reduce(function (sum, row) {
      return sum + row.subtotal;
    }, 0);
    var difference = totalAmount - totalCalculated;
    return {
      rows: rows,
      totalCalculated: totalCalculated,
      difference: difference,
      isBalanced: difference === 0
    };
  }

  function adjustDifference(session, result) {
    var targetIndex = 0;
    result.rows.forEach(function (row, index) {
      if (row.ratio > result.rows[targetIndex].ratio) {
        targetIndex = index;
      }
    });

    var rows = result.rows.map(function (row) {
      return Object.assign({}, row);
    });
    var target = rows[targetIndex];
    var adjustedSubtotal = target.subtotal + result.difference;
    var adjustedAmountPerPerson = adjustedSubtotal / target.count;

    if (!Number.isInteger(adjustedAmountPerPerson)) {
      return result;
    }

    target.amountPerPerson = adjustedAmountPerPerson;
    target.subtotal = adjustedSubtotal;
    return buildCalculationResult(session.totalAmount, rows);
  }

  function createCollectionMembers(result) {
    var members = [];
    result.rows.forEach(function (row) {
      for (var index = 1; index <= row.count; index += 1) {
        members.push({
          id: row.groupId + "-member-" + index,
          label: row.groupName + " " + index,
          groupId: row.groupId,
          groupName: row.groupName,
          amount: row.amountPerPerson,
          collected: false
        });
      }
    });
    return members;
  }

  function summarizeCollection(members) {
    var plannedTotal = members.reduce(function (sum, member) {
      return sum + member.amount;
    }, 0);
    var collectedTotal = members.reduce(function (sum, member) {
      return sum + (member.collected ? member.amount : 0);
    }, 0);
    var collectedCount = members.filter(function (member) {
      return member.collected;
    }).length;

    return {
      plannedTotal: plannedTotal,
      collectedTotal: collectedTotal,
      uncollectedTotal: plannedTotal - collectedTotal,
      collectedCount: collectedCount,
      uncollectedCount: members.length - collectedCount
    };
  }

  function formatCurrency(value) {
    return Math.round(value).toLocaleString("ja-JP") + "円";
  }

  function formatCount(value) {
    return value.toLocaleString("ja-JP") + "人";
  }

  function readNumberInput(id) {
    var element = document.getElementById(id);
    return Number(element.value);
  }

  function syncStateFromForm() {
    state.totalAmount = readNumberInput("total-amount");
    state.roundingUnit = readNumberInput("rounding-unit");
    state.groups = Array.from(document.querySelectorAll(".group-card")).map(function (card) {
      return {
        id: card.dataset.groupId,
        name: card.querySelector(".group-name").value.trim(),
        ratio: Number(card.querySelector(".group-ratio").value),
        count: Number(card.querySelector(".group-count").value)
      };
    });
  }

  function resetComputedState() {
    if (state.calculationResult || state.collectionMembers.length > 0) {
      state.calculationResult = null;
      state.collectionMembers = [];
      renderResults();
      renderCollection();
      setNotice("入力内容を変更すると、計算結果と集金記録はリセットされます。", false);
    }
  }

  function setNotice(message, isError) {
    var notice = document.getElementById("notice");
    notice.textContent = message;
    notice.classList.toggle("error", Boolean(isError));
  }

  function renderGroups() {
    var container = document.getElementById("groups");
    var template = document.getElementById("group-template");
    container.innerHTML = "";

    state.groups.forEach(function (group, index) {
      var node = template.content.firstElementChild.cloneNode(true);
      node.dataset.groupId = group.id;
      node.querySelector("h4").textContent = "区分 " + (index + 1);
      node.querySelector(".group-name").value = group.name;
      node.querySelector(".group-ratio").value = String(group.ratio);
      node.querySelector(".group-count").value = String(group.count);
      node.querySelector(".danger-button").addEventListener("click", function () {
        if (state.groups.length <= 1) {
          setNotice("負担区分は1件以上必要です。", true);
          return;
        }
        syncStateFromForm();
        state.groups = state.groups.filter(function (item) {
          return item.id !== group.id;
        });
        resetComputedState();
        renderGroups();
      });
      Array.from(node.querySelectorAll("input")).forEach(function (input) {
        input.addEventListener("input", function () {
          syncStateFromForm();
          resetComputedState();
        });
      });
      container.appendChild(node);
    });
  }

  function renderResults() {
    var content = document.getElementById("result-content");
    var badge = document.getElementById("balance-badge");

    if (!state.calculationResult) {
      content.className = "empty-state";
      content.textContent = "計算すると区分ごとの金額を表示します。";
      badge.className = "badge muted";
      badge.textContent = "未計算";
      return;
    }

    var result = state.calculationResult;
    badge.className = result.isBalanced ? "badge ok" : "badge warn";
    badge.textContent = result.isBalanced ? "計算済み" : "差額あり";
    content.className = "";

    content.innerHTML = [
      '<table class="result-table">',
      "<thead><tr><th>区分名</th><th>1人あたり</th><th>人数</th><th>小計</th></tr></thead>",
      "<tbody>",
      result.rows.map(function (row) {
        return "<tr><td>" + escapeHtml(row.groupName) + '</td><td class="number">' + formatCurrency(row.amountPerPerson) + '</td><td class="number">' + formatCount(row.count) + '</td><td class="number">' + formatCurrency(row.subtotal) + "</td></tr>";
      }).join(""),
      "</tbody></table>",
      '<div class="verify-box"><span>合計検証</span><span class="' + (result.isBalanced ? "badge ok" : "badge warn") + '">' + (result.isBalanced ? "OK" : "差額あり " + formatCurrency(result.difference)) + "</span></div>",
      '<div class="total-box"><span>結果: 合計</span><strong>' + formatCurrency(result.totalCalculated) + "</strong></div>",
      '<button id="start-collection" class="primary-button" type="button">集金管理へ進む</button>'
    ].join("");

    document.getElementById("start-collection").addEventListener("click", function () {
      state.collectionMembers = createCollectionMembers(state.calculationResult);
      renderCollection();
      switchScreen("collection");
      setNotice("集金管理を開始しました。", false);
    });
  }

  function renderCollection() {
    var content = document.getElementById("collection-content");
    if (!state.calculationResult) {
      content.className = "empty-state";
      content.textContent = "集金管理を開始するには、先にワリカン計算を行ってください。";
      return;
    }
    if (state.collectionMembers.length === 0) {
      content.className = "empty-state";
      content.textContent = "計算結果から集金対象者を生成してください。";
      return;
    }

    var summary = summarizeCollection(state.collectionMembers);
    content.className = "";
    content.innerHTML = [
      '<div class="summary-grid">',
      '<div class="summary-card"><span>合計予定額</span><strong>' + formatCurrency(summary.plannedTotal) + "</strong></div>",
      '<div class="summary-card"><span>集金済み</span><strong>' + formatCurrency(summary.collectedTotal) + "</strong></div>",
      '<div class="summary-card"><span>未集金</span><strong>' + formatCurrency(summary.uncollectedTotal) + "</strong></div>",
      "</div>",
      '<table class="collection-table">',
      "<thead><tr><th>名前 / 区分</th><th>金額</th><th>状態</th><th>操作</th></tr></thead>",
      "<tbody>",
      state.collectionMembers.map(function (member) {
        return '<tr><td>' + escapeHtml(member.label) + '</td><td class="number">' + formatCurrency(member.amount) + '</td><td><span class="' + (member.collected ? "status-collected" : "status-open") + '">' + (member.collected ? "集金済み" : "未集金") + '</span></td><td><button class="' + (member.collected ? "toggle-button secondary" : "toggle-button") + '" type="button" data-member-id="' + member.id + '">' + (member.collected ? "未集金に戻す" : "集金済みにする") + "</button></td></tr>";
      }).join(""),
      "</tbody></table>"
    ].join("");

    Array.from(content.querySelectorAll("[data-member-id]")).forEach(function (button) {
      button.addEventListener("click", function () {
        var id = button.dataset.memberId;
        state.collectionMembers = state.collectionMembers.map(function (member) {
          if (member.id !== id) {
            return member;
          }
          return Object.assign({}, member, { collected: !member.collected });
        });
        renderCollection();
      });
    });
  }

  function escapeHtml(value) {
    return String(value).replace(/[&<>"']/g, function (char) {
      return {
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#039;"
      }[char];
    });
  }

  function switchScreen(screen) {
    document.getElementById("screen-calc").classList.toggle("active", screen === "calc");
    document.getElementById("screen-collection").classList.toggle("active", screen === "collection");
    document.getElementById("tab-calc").classList.toggle("active", screen === "calc");
    document.getElementById("tab-collection").classList.toggle("active", screen === "collection");
  }

  function initBrowser() {
    document.getElementById("total-amount").value = String(state.totalAmount);
    document.getElementById("rounding-unit").value = String(state.roundingUnit);
    renderGroups();
    renderResults();
    renderCollection();

    ["total-amount", "rounding-unit"].forEach(function (id) {
      document.getElementById(id).addEventListener("input", function () {
        syncStateFromForm();
        resetComputedState();
      });
    });

    document.getElementById("add-group").addEventListener("click", function () {
      syncStateFromForm();
      state.groups.push({
        id: "group-" + nextGroupId,
        name: "",
        ratio: 1.0,
        count: 1
      });
      nextGroupId += 1;
      resetComputedState();
      renderGroups();
    });

    document.getElementById("calc-form").addEventListener("submit", function (event) {
      event.preventDefault();
      syncStateFromForm();
      var calculation = calculateWarikan(state);
      if (calculation.errors.length > 0) {
        setNotice("入力内容を確認してください " + calculation.errors.join(" "), true);
        return;
      }
      state.calculationResult = calculation.result;
      state.collectionMembers = [];
      renderResults();
      renderCollection();
      setNotice("ワリカン額を計算しました", false);
    });

    Array.from(document.querySelectorAll(".tab")).forEach(function (tab) {
      tab.addEventListener("click", function () {
        switchScreen(tab.dataset.screen);
      });
    });
  }

  if (typeof document !== "undefined") {
    document.addEventListener("DOMContentLoaded", initBrowser);
  }

  if (typeof module !== "undefined") {
    module.exports = {
      calculateWarikan: calculateWarikan,
      createCollectionMembers: createCollectionMembers,
      summarizeCollection: summarizeCollection,
      validateSession: validateSession,
      formatCurrency: formatCurrency,
      formatCount: formatCount
    };
  }
}());
